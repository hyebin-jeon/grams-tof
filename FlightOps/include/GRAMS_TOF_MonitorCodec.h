#pragma once

#include "GRAMS_TOF_CommandCodec.h" // Depend on the standard Packet structure
#include <vector>
#include <cstdint>

class GRAMS_TOF_MonitorCodec {
public:
    // This represents the MONITORING DATA layout inside the standard argv
    struct MonitorData {
        uint32_t run_number;
        char     hname[16];   // Primary identifier
        uint32_t hist_type;   // 1 for TH1F, 2 for TH2F, 3 for TProfile
        uint32_t n_bins_x;
        float    x_min;          
        float    x_max;          
        uint32_t n_bins_y;
        float    y_min;          
        float    y_max;          
        std::vector<uint32_t> bins;
    };

    //brief Converts MonitorData into a standard CommandCodec::Packet.
    static GRAMS_TOF_CommandCodec::Packet encode(const MonitorData& data);

    
    //@brief Extracts MonitorData from a standard CommandCodec::Packet.
    static bool decode(const GRAMS_TOF_CommandCodec::Packet& packet, MonitorData& outData);
};

