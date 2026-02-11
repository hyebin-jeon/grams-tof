#include "GRAMS_TOF_MonitorCodec.h"
#include <cstring>

GRAMS_TOF_CommandCodec::Packet GRAMS_TOF_MonitorCodec::encode(const MonitorData& data) {
    GRAMS_TOF_CommandCodec::Packet packet;
    packet.code = static_cast<uint16_t>(TOFCommandCode::MONITOR_DATA_STREAM); 

    packet.argv.push_back(data.run_number); // [0]

    // Pack 16-byte hname into 4 uint32_t slots [1-4]
    uint32_t name_buf[4];
    std::memcpy(name_buf, data.hname, 16);
    for(int i=0; i<4; ++i) packet.argv.push_back(name_buf[i]);

    packet.argv.push_back(data.hist_type);  // [5]
    packet.argv.push_back(data.n_bins_x);   // [6]
    
    uint32_t u_xmin, u_xmax;
    std::memcpy(&u_xmin, &data.x_min, 4);
    std::memcpy(&u_xmax, &data.x_max, 4);
    packet.argv.push_back(u_xmin);          // [7]
    packet.argv.push_back(u_xmax);          // [8]

    packet.argv.push_back(data.n_bins_y);   // [9]
    
    uint32_t u_ymin, u_ymax;
    std::memcpy(&u_ymin, &data.y_min, 4);
    std::memcpy(&u_ymax, &data.y_max, 4);
    packet.argv.push_back(u_ymin);          // [10]
    packet.argv.push_back(u_ymax);          // [11]

    packet.argv.insert(packet.argv.end(), data.bins.begin(), data.bins.end());
    packet.argc = static_cast<uint16_t>(packet.argv.size());
    return packet;
}

bool GRAMS_TOF_MonitorCodec::decode(const GRAMS_TOF_CommandCodec::Packet& packet, MonitorData& outData) {
    // Minimum 12 words for the header
    if (packet.argv.size() < 12) return false;

    outData.run_number = packet.argv[0];
    std::memcpy(outData.hname, &packet.argv[1], 16);
    
    outData.hist_type  = packet.argv[5];
    outData.n_bins_x   = packet.argv[6];
    
    std::memcpy(&outData.x_min, &packet.argv[7], 4);
    std::memcpy(&outData.x_max, &packet.argv[8], 4);

    outData.n_bins_y   = packet.argv[9];
    std::memcpy(&outData.y_min, &packet.argv[10], 4);
    std::memcpy(&outData.y_max, &packet.argv[11], 4);

    outData.bins.assign(packet.argv.begin() + 12, packet.argv.end());
    return true;
}

