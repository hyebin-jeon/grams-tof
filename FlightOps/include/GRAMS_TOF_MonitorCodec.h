#pragma once

#include "GRAMS_TOF_CommandCodec.h"
#include <vector>
#include <cstdint>

class GRAMS_TOF_MonitorCodec {
public:
    // hist_type / data_type values shared across MonitorData, GraphData, ParameterData
    enum class DataType : uint32_t {
        TH1F      = 1,
        TH2F      = 2,
        TProfile  = 3,
        TGraph    = 4,
        TParameter = 5,
    };

    // ── Histogram (TH1F / TH2F / TProfile) ──────────────────────────────
    struct MonitorData {
        uint32_t run_number;
        char     hname[16];
        uint32_t hist_type;   // DataType::TH1F / TH2F / TProfile
        uint32_t n_bins_x;
        float    x_min;
        float    x_max;
        uint32_t n_bins_y;
        float    y_min;
        float    y_max;
        std::vector<uint32_t> bins;
    };

    // ── TGraph ───────────────────────────────────────────────────────────
    // argv layout:
    //   [0]    run_number
    //   [1-4]  gname (16 bytes)
    //   [5]    data_type = DataType::TGraph
    //   [6]    n_points
    //   [7+]   X/Y pairs packed as double (2×uint32 hi/lo each):
    //          X0_hi, X0_lo, Y0_hi, Y0_lo, X1_hi, X1_lo, Y1_hi, Y1_lo, ...
    struct GraphData {
        uint32_t run_number;
        char     gname[16];
        uint32_t n_points;
        std::vector<double> x; // X values
        std::vector<double> y; // Y values (e.g. UNIX timestamp)
    };

    // ── TParameter<double> ───────────────────────────────────────────────
    // argv layout:
    //   [0]    run_number
    //   [1-4]  pname (16 bytes)
    //   [5]    data_type = DataType::TParameter
    //   [6-7]  value (double packed as two uint32_t, big-endian hi/lo)
    struct ParameterData {
        uint32_t run_number;
        char     pname[16];
        double   value;
    };

    // Histogram encode/decode (unchanged interface)
    static GRAMS_TOF_CommandCodec::Packet encode(const MonitorData& data);
    static bool decode(const GRAMS_TOF_CommandCodec::Packet& packet, MonitorData& outData);

    // Graph encode/decode
    static GRAMS_TOF_CommandCodec::Packet encode(const GraphData& data);
    static bool decode(const GRAMS_TOF_CommandCodec::Packet& packet, GraphData& outData);

    // Parameter encode/decode
    static GRAMS_TOF_CommandCodec::Packet encode(const ParameterData& data);
    static bool decode(const GRAMS_TOF_CommandCodec::Packet& packet, ParameterData& outData);
};
