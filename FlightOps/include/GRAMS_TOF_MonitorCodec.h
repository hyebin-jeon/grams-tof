#pragma once

#include <vector>
#include <cstdint>

class GRAMS_TOF_MonitorCodec {
public:
    struct Packet {
        uint32_t run_number;
        uint32_t hist_type;   // 1 for TH1F, 2 for TH2F
        uint32_t n_bins_x;
        uint32_t n_bins_y;
        std::vector<uint32_t> argv; // The histogram bins
    };

    //brief Serialize the monitor packet into a raw byte stream.
    static std::vector<uint8_t> serialize(const Packet& packet);

    //brief Parse raw bytes back into a Monitor Packet 
    static bool parse(const std::vector<uint8_t>& data, Packet& outPacket);
};

