#include "GRAMS_TOF_LogCodec.h"
#include "GRAMS_TOF_Logger.h"
#include <cstring>
#include <algorithm>

constexpr size_t MAX_LOG_LENGTH = 10000; // 10k character limit

GRAMS_TOF_CommandCodec::Packet GRAMS_TOF_LogCodec::encode(const LogData& data) {
    GRAMS_TOF_CommandCodec::Packet packet;
    packet.code = static_cast<uint16_t>(TOFCommandCode::LOGGER_DATA_STREAM);

    // Dynamic message clamping to 10k characters
    std::string safe_msg = data.message;
    if (safe_msg.size() > MAX_LOG_LENGTH) {
        safe_msg = safe_msg.substr(0, MAX_LOG_LENGTH) + "... [TRUNCATED]";
    }
    uint32_t msg_len = static_cast<uint32_t>(safe_msg.size());

    // Header layout (9 words total):
    // [0] run_number
    packet.argv.push_back(data.run_number);

    // [1-2] timestamp_ms (split uint64_t into two uint32_t words)
    packet.argv.push_back(static_cast<uint32_t>((data.timestamp_ms >> 32) & 0xFFFFFFFF));
    packet.argv.push_back(static_cast<uint32_t>(data.timestamp_ms & 0xFFFFFFFF));

    // [3] level
    packet.argv.push_back(static_cast<uint32_t>(data.level));

    // [4] exact string length in bytes
    packet.argv.push_back(msg_len);

    // [5-8] component string (16 bytes packed into 4 x uint32_t)
    uint32_t comp_buf[4] = {0};
    std::memcpy(comp_buf, data.component, std::min(sizeof(data.component), size_t(16)));
    for (int i = 0; i < 4; ++i) {
        packet.argv.push_back(comp_buf[i]);
    }

    // [9..N] message characters packed in 4-byte chunks
    size_t num_words = (msg_len + 3) / 4;
    for (size_t i = 0; i < num_words; ++i) {
        uint32_t word = 0;
        size_t bytes_to_copy = std::min(size_t(4), msg_len - (i * 4));
        std::memcpy(&word, safe_msg.data() + (i * 4), bytes_to_copy);
        packet.argv.push_back(word);
    }

    packet.argc = static_cast<uint16_t>(packet.argv.size());
    return packet;
}

bool GRAMS_TOF_LogCodec::decode(const GRAMS_TOF_CommandCodec::Packet& packet, LogData& outData) {
    // Minimum 9 words for header
    if (packet.argv.size() < 9) {
        return false;
    }

    outData.run_number = packet.argv[0];
    outData.timestamp_ms = (static_cast<uint64_t>(packet.argv[1]) << 32) | packet.argv[2];
    outData.level = static_cast<uint8_t>(packet.argv[3] & 0xFF);
    
    uint32_t msg_len = packet.argv[4];

    // Verify vector has enough uint32_t words to hold msg_len bytes
    size_t required_words = 9 + (msg_len + 3) / 4;
    if (packet.argv.size() < required_words) {
        return false; // Corrupted packet or incomplete payload
    }

    // Safely copy component string memory
    std::memset(outData.component, 0, sizeof(outData.component));
    std::memcpy(outData.component, static_cast<const void*>(&packet.argv[5]), 16);
    outData.component[15] = '\0'; // Ensure string termination

    // Safely unpack message string from packet memory
    outData.message.clear();
    if (msg_len > 0) {
        outData.message.resize(msg_len);
        std::memcpy(&outData.message[0], static_cast<const void*>(&packet.argv[9]), msg_len);
    }

    return true;
}
