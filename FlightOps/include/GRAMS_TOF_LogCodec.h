#pragma once

#include "GRAMS_TOF_CommandCodec.h"
#include <vector>
#include <string>
#include <cstdint>

class GRAMS_TOF_LogCodec {
public:
    struct LogData {
        uint32_t run_number{0};
        uint64_t timestamp_ms{0}; // Unix epoch in milliseconds
        uint8_t  level{2};        // 0=Detail, 1=Debug, 2=Info, 3=Warn, 4=Err, 5=Crit
        char     component[16]{}; // Fixed 16-byte component tag
        std::string message;      // Arbitrary-length text message
    };

    /// @brief Encodes LogData into a standard CommandCodec::Packet
    static GRAMS_TOF_CommandCodec::Packet encode(const LogData& data);

    /// @brief Decodes a CommandCodec::Packet back into LogData
    static bool decode(const GRAMS_TOF_CommandCodec::Packet& packet, LogData& outData);
};
