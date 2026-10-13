#include "GRAMS_TOF_MonitorCodec.h"
#include "GRAMS_TOF_Logger.h"
#include <cstring>

// ─────────────────────────────────────────────────────────────
//  Internal helpers
// ─────────────────────────────────────────────────────────────

static void pushName(GRAMS_TOF_CommandCodec::Packet& packet, const char* name, size_t len = 16) {
    uint32_t buf[4] = {};
    std::memcpy(buf, name, len);
    for (int i = 0; i < 4; ++i) packet.argv.push_back(buf[i]);
}

static void copyName(char* dst, const GRAMS_TOF_CommandCodec::Packet& packet, int startIdx, size_t len = 16) {
    std::memcpy(dst, &packet.argv[startIdx], len);
}

// ─────────────────────────────────────────────────────────────
//  MonitorData (TH1F / TH2F / TProfile)
// ─────────────────────────────────────────────────────────────

GRAMS_TOF_CommandCodec::Packet GRAMS_TOF_MonitorCodec::encode(const MonitorData& data) {
    GRAMS_TOF_CommandCodec::Packet packet;
    packet.code = static_cast<uint16_t>(TOFCommandCode::MONITOR_DATA_STREAM);

    packet.argv.push_back(data.run_number);  // [0]
    pushName(packet, data.hname);            // [1-4]
    packet.argv.push_back(data.hist_type);   // [5]  DataType::TH1F/TH2F/TProfile
    packet.argv.push_back(data.n_bins_x);    // [6]

    uint32_t u_xmin, u_xmax;
    std::memcpy(&u_xmin, &data.x_min, 4);
    std::memcpy(&u_xmax, &data.x_max, 4);
    packet.argv.push_back(u_xmin);           // [7]
    packet.argv.push_back(u_xmax);           // [8]

    packet.argv.push_back(data.n_bins_y);    // [9]

    uint32_t u_ymin, u_ymax;
    std::memcpy(&u_ymin, &data.y_min, 4);
    std::memcpy(&u_ymax, &data.y_max, 4);
    packet.argv.push_back(u_ymin);           // [10]
    packet.argv.push_back(u_ymax);           // [11]

    packet.argv.insert(packet.argv.end(), data.bins.begin(), data.bins.end()); // [12+]
    packet.argc = static_cast<uint16_t>(packet.argv.size());
    return packet;
}

bool GRAMS_TOF_MonitorCodec::decode(const GRAMS_TOF_CommandCodec::Packet& packet, MonitorData& outData) {
    if (packet.argv.size() < 12) {
        Logger::instance().error(
            "[MonitorCodec] Truncated monitoring stream packet received. "
            "Size {} is less than header layout minimum (12).", packet.argv.size());
        return false;
    }

    outData.run_number = packet.argv[0];
    copyName(outData.hname, packet, 1);

    outData.hist_type = packet.argv[5];
    outData.n_bins_x  = packet.argv[6];
    std::memcpy(&outData.x_min, &packet.argv[7], 4);
    std::memcpy(&outData.x_max, &packet.argv[8], 4);
    outData.n_bins_y  = packet.argv[9];
    std::memcpy(&outData.y_min, &packet.argv[10], 4);
    std::memcpy(&outData.y_max, &packet.argv[11], 4);

    outData.bins.assign(packet.argv.begin() + 12, packet.argv.end());
    return true;
}

// ─────────────────────────────────────────────────────────────
//  GraphData (TGraph)
// ─────────────────────────────────────────────────────────────

GRAMS_TOF_CommandCodec::Packet GRAMS_TOF_MonitorCodec::encode(const GraphData& data) {
    GRAMS_TOF_CommandCodec::Packet packet;
    packet.code = static_cast<uint16_t>(TOFCommandCode::MONITOR_DATA_STREAM);

    packet.argv.push_back(data.run_number);                          // [0]
    pushName(packet, data.gname);                                    // [1-4]
    packet.argv.push_back(static_cast<uint32_t>(DataType::TGraph)); // [5]
    packet.argv.push_back(data.n_points);                            // [6]

    // Pack each X/Y pair as 4 uint32_t (hi/lo for each double)
    for (uint32_t i = 0; i < data.n_points; ++i) {
        uint64_t bx, by;
        std::memcpy(&bx, &data.x[i], sizeof(double));
        std::memcpy(&by, &data.y[i], sizeof(double));
        packet.argv.push_back(static_cast<uint32_t>(bx >> 32));          // X hi
        packet.argv.push_back(static_cast<uint32_t>(bx & 0xFFFFFFFF));  // X lo
        packet.argv.push_back(static_cast<uint32_t>(by >> 32));          // Y hi
        packet.argv.push_back(static_cast<uint32_t>(by & 0xFFFFFFFF));  // Y lo
    }

    packet.argc = static_cast<uint16_t>(packet.argv.size());
    return packet;
}

bool GRAMS_TOF_MonitorCodec::decode(const GRAMS_TOF_CommandCodec::Packet& packet, GraphData& outData) {
    // Minimum: run(1) + name(4) + type(1) + n_points(1) = 7 words
    if (packet.argv.size() < 7) {
        Logger::instance().error(
            "[MonitorCodec] Truncated TGraph packet. Size {}.", packet.argv.size());
        return false;
    }
    if (packet.argv[5] != static_cast<uint32_t>(DataType::TGraph)) {
        Logger::instance().error(
            "[MonitorCodec] decode(GraphData) called on non-TGraph packet (data_type={}).",
            packet.argv[5]);
        return false;
    }

    outData.run_number = packet.argv[0];
    copyName(outData.gname, packet, 1);
    outData.n_points = packet.argv[6];

    outData.x.resize(outData.n_points);
    outData.y.resize(outData.n_points);
    for (uint32_t i = 0; i < outData.n_points; ++i) {
        uint64_t bx = (static_cast<uint64_t>(packet.argv[7 + i*4])     << 32)
                    |  static_cast<uint64_t>(packet.argv[7 + i*4 + 1]);
        uint64_t by = (static_cast<uint64_t>(packet.argv[7 + i*4 + 2]) << 32)
                    |  static_cast<uint64_t>(packet.argv[7 + i*4 + 3]);
        std::memcpy(&outData.x[i], &bx, sizeof(double));
        std::memcpy(&outData.y[i], &by, sizeof(double));
    }
    return true;
}

// ─────────────────────────────────────────────────────────────
//  ParameterData (TParameter<double>)
// ─────────────────────────────────────────────────────────────

GRAMS_TOF_CommandCodec::Packet GRAMS_TOF_MonitorCodec::encode(const ParameterData& data) {
    GRAMS_TOF_CommandCodec::Packet packet;
    packet.code = static_cast<uint16_t>(TOFCommandCode::MONITOR_DATA_STREAM);

    packet.argv.push_back(data.run_number);                                      // [0]
    pushName(packet, data.pname);                                                // [1-4]
    packet.argv.push_back(static_cast<uint32_t>(DataType::TParameter));         // [5]

    // Pack double (8 bytes) as two uint32_t (hi word first)
    uint64_t bits;
    std::memcpy(&bits, &data.value, sizeof(double));
    packet.argv.push_back(static_cast<uint32_t>(bits >> 32));  // [6] hi
    packet.argv.push_back(static_cast<uint32_t>(bits & 0xFFFFFFFF)); // [7] lo

    packet.argc = static_cast<uint16_t>(packet.argv.size());
    return packet;
}

bool GRAMS_TOF_MonitorCodec::decode(const GRAMS_TOF_CommandCodec::Packet& packet, ParameterData& outData) {
    // Minimum: run(1) + name(4) + type(1) + value_hi(1) + value_lo(1) = 8 words
    if (packet.argv.size() < 8) {
        Logger::instance().error(
            "[MonitorCodec] Truncated TParameter packet. Size {}.", packet.argv.size());
        return false;
    }
    if (packet.argv[5] != static_cast<uint32_t>(DataType::TParameter)) {
        Logger::instance().error(
            "[MonitorCodec] decode(ParameterData) called on non-TParameter packet (data_type={}).",
            packet.argv[5]);
        return false;
    }

    outData.run_number = packet.argv[0];
    copyName(outData.pname, packet, 1);

    uint64_t bits = (static_cast<uint64_t>(packet.argv[6]) << 32)
                  |  static_cast<uint64_t>(packet.argv[7]);
    std::memcpy(&outData.value, &bits, sizeof(double));
    return true;
}
