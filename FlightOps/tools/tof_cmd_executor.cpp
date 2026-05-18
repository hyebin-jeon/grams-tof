#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "GRAMS_TOF_CommandCodec.h"
#include "GRAMS_TOF_CommandDefs.h"

// Helper to setup a listening socket (Logic from test_grams_tof_client_daq.cpp)
int setupServer(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    // Fixes the "Port is in TIME_WAIT" issue
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

    // Fixes the "Multiple processes trying to bind" issue
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEPORT) failed");
    }
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) { 
      close(sock);
      return -1;
    }
    listen(sock, 1);
    return sock;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <CommandCodeHex> [Args...]" << std::endl;
        return 1;
    }

    uint16_t targetCode = static_cast<uint16_t>(std::stoul(argv[1], nullptr, 16));
    std::vector<uint32_t> cmdArgs;
    for (int i = 2; i < argc; ++i) cmdArgs.push_back(static_cast<uint32_t>(std::stoul(argv[i])));

    // 1. Prepare BOTH servers to satisfy the DAQ Client's dual-port requirement
    int cmdServer = setupServer(50007);
    int evtServer = setupServer(50006);
    
    if (cmdServer < 0 || evtServer < 0) {
        std::cerr << "Failed to open ports 50006 or 50007." << std::endl;
        return 1;
    }

    std::cout << ">>> Waiting for DAQ Client to connect to BOTH ports..." << std::endl;

    // 2. Synchronize: Wait for both connections before sending the command
    int evtSock = accept(evtServer, nullptr, nullptr);
    std::cout << ">>> Event connection (50006) established." << std::endl;
    
    int cmdSock = accept(cmdServer, nullptr, nullptr);
    std::cout << ">>> Command connection (50007) established." << std::endl;

    // 3. Construct and send the packet on the Command Port (50007)
    GRAMS_TOF_CommandCodec::Packet pkt;
    pkt.code = targetCode;
    pkt.argv = cmdArgs;
    pkt.argc = static_cast<uint16_t>(cmdArgs.size());
    auto data = GRAMS_TOF_CommandCodec::serialize(pkt);
    
    std::cout << ">>> Sending command 0x" << std::hex << targetCode << "..." << std::endl;
    send(cmdSock, data.data(), data.size(), 0);

    // 4. Wait for Callback on the EVENT Port (50006)
    // As per GRAMS_TOF_CommandDispatch.cpp, status callbacks are sent via eventClient_
    const uint16_t CALLBACK_VAL = 0x5FFE; 
    std::vector<uint8_t> buffer;
    uint8_t rawBuf[8192]; // Large buffer to handle potential science data bursts

    std::cout << ">>> Waiting for Callback (0x5FFE) for command 0x" << std::hex << targetCode << "..." << std::endl;
    
    while (true) {
        ssize_t n = recv(evtSock, rawBuf, sizeof(rawBuf), 0);
        if (n <= 0) {
            std::cerr << ">>> Connection lost." << std::endl;
            break;
        }
    
        buffer.insert(buffer.end(), rawBuf, rawBuf + n);
    
        GRAMS_TOF_CommandCodec::Packet respPkt;
        // Use the codec to parse the incoming stream
        if (GRAMS_TOF_CommandCodec::parse(buffer, respPkt)) {
            
            // 1. Check if the packet is a Callback (0x5FFE)
            if (respPkt.code == 0x5FFE) { 
                // 2. Extract which command this callback is reporting on
                uint16_t reportedCmd = static_cast<uint16_t>(respPkt.argv[0]);
                uint32_t status = respPkt.argv[1];
   
                if (reportedCmd == targetCode) {
                    std::cout << ">>> MATCHED CALLBACK: Command 0x" << std::hex << reportedCmd
                              << " finished with status " << std::dec << status << std::endl;
                
                    // --- NEW: AUTOMATIC 0x5003 WRAPPER ---
                    // If the command we just finished was NOT 0x5003 itself, trigger a reset
                    if (targetCode != 0x5003) {
                        std::cout << ">>> Auto-Reset: Sending RECONNECT_NETWORK (0x5003)..." << std::endl;
                        
                        GRAMS_TOF_CommandCodec::Packet resetPkt;
                        resetPkt.code = 0x5003; 
                        resetPkt.argc = 0;
                        auto resetData = GRAMS_TOF_CommandCodec::serialize(resetPkt);
                        
                        // Send to the command port
                        send(cmdSock, resetData.data(), resetData.size(), 0);
                        
                        // Give the C++ Client a tiny moment to process the packet 
                        // before we close our end of the pipe.
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    }
                    // -------------------------------------
                
                    // Cleanup and exit
                    std::cout << ">>> Closing executor and releasing ports..." << std::endl;
                    close(cmdSock); 
                    close(evtSock);
                    close(cmdServer);
                    close(evtServer);
                    
                    return (status == 0) ? 0 : 1;
                } else {
                    // This is an intermediate step callback (common in Macros)
                    std::cout << ">>> IGNORED: Intermediate callback for step 0x" << std::hex << reportedCmd << std::endl;
                }
            }
            
            // Clear processed packet from the buffer
            buffer.erase(buffer.begin(), buffer.begin() + GRAMS_TOF_CommandCodec::serialize(respPkt).size());
        }
    }

    return 1;
}
