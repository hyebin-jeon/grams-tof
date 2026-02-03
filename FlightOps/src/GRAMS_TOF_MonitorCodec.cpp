#include "GRAMS_TOF_MonitorCodec.h"
#include <cstring> 

std::vector<uint8_t> GRAMS_TOF_MonitorCodec::serialize(const Packet& packet) {
    // 1. Calculate the size of the header (4 fields * 4 bytes = 16 bytes)
    const size_t headerSize = 4 * sizeof(uint32_t);
    // 2. Calculate size of the data payload
    const size_t dataSize = packet.argv.size() * sizeof(uint32_t);
    
    std::vector<uint8_t> buffer(headerSize + dataSize);

    // 3. Serialize Header fields
    uint32_t header[4] = {
        packet.run_number,
        packet.hist_type,
        packet.n_bins_x,
        packet.n_bins_y
    };
    std::memcpy(buffer.data(), header, headerSize);

    // 4. Serialize Data (argv)
    if (dataSize > 0) {
        std::memcpy(buffer.data() + headerSize, packet.argv.data(), dataSize);
    }

    return buffer;
}

bool GRAMS_TOF_MonitorCodec::parse(const std::vector<uint8_t>& data, Packet& outPacket) {
    const size_t headerSize = 4 * sizeof(uint32_t);
    
    // Basic validation: must at least contain the header
    if (data.size() < headerSize) {
        return false;
    }

    // 1. Parse Header
    const uint32_t* headerPtr = reinterpret_cast<const uint32_t*>(data.data());
    outPacket.run_number = headerPtr[0];
    outPacket.hist_type  = headerPtr[1];
    outPacket.n_bins_x   = headerPtr[2];
    outPacket.n_bins_y   = headerPtr[3];

    // 2. Parse Data (remaining bytes)
    size_t dataSizeBytes = data.size() - headerSize;
    size_t numElements = dataSizeBytes / sizeof(uint32_t);
    
    outPacket.argv.resize(numElements);
    if (numElements > 0) {
        std::memcpy(outPacket.argv.data(), data.data() + headerSize, dataSizeBytes);
    }

    return true;
}
