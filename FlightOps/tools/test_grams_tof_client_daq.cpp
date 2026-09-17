#include <ncurses.h>
#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <atomic>
#include <chrono>
#include <iostream>
#include <vector>
#include <sstream> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <fcntl.h>
#include <iomanip>
#include <condition_variable>
#include <mosquitto.h>

#include "GRAMS_TOF_CommandDefs.h"
#include "GRAMS_TOF_CommandCodec.h"
#include "GRAMS_TOF_LogCodec.h"

// ---------- Global variables ----------
std::mutex logMutex;
std::queue<std::string> logQueue;
std::atomic<bool> running{true};
std::atomic<int> heartbeatInterval{1};

std::atomic<int> commandClientSock{-1};
std::atomic<int> eventClientSock{-1};

std::mutex callbackMutex;
std::condition_variable callbackCv;
uint16_t activeTargetCode = 0;
uint16_t lastFinishedCmd = 0;
uint32_t lastCmdStatus = 0;

struct mosquitto* g_mosq = nullptr;

// ---------- Logging helper ----------
void addLog(const std::string& msg) {
    std::lock_guard<std::mutex> lock(logMutex);
    logQueue.push(msg);
}

void logPacketSent(const GRAMS_TOF_CommandCodec::Packet& pkt) {
    std::stringstream ss;
    ss << "[Client] Sent command 0x";
    ss << std::hex << std::uppercase << pkt.code;
    ss << " with " << std::dec << pkt.argc << " args";
    if (pkt.argc > 0) {
        ss << " [Args: ";
        for (size_t i = 0; i < pkt.argc; ++i) {
            ss << pkt.argv[i] << (i == pkt.argc - 1 ? "" : ", ");
        }
        ss << "]";
    }
    addLog(ss.str());
}

ssize_t readAll(int sock, char* buffer, size_t len) {
    size_t totalRead = 0;
    while (totalRead < len) {
        ssize_t n = recv(sock, buffer + totalRead, len - totalRead, 0);

        if (n == 0) return 0;
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        totalRead += n;
    }
    return totalRead;
}

// --- Function to build a packet ---
GRAMS_TOF_CommandCodec::Packet buildPacket(uint16_t code, const std::vector<int>& args) {
    GRAMS_TOF_CommandCodec::Packet pkt;
    pkt.code = code;
    pkt.argv.clear();
    for (auto a : args) pkt.argv.push_back(static_cast<int32_t>(a));
    pkt.argc = static_cast<uint16_t>(pkt.argv.size());
    return pkt;
}

// --- Function to send a packet ---
void sendPacket(int sock, const GRAMS_TOF_CommandCodec::Packet& pkt) {
    auto data = GRAMS_TOF_CommandCodec::serialize(pkt);
    size_t totalSent = 0;
    while (totalSent < data.size()) {
        ssize_t sent = send(sock, data.data() + totalSent, data.size() - totalSent, 0);
        if (sent < 0) throw std::runtime_error("Send failed");
        totalSent += sent;
    }
}

// Helper to convert LogLevel byte to String
std::string getLogLevelString(uint8_t level) {
    switch (level) {
        case 0: return "TRACE";
        case 1: return "DEBUG";
        case 2: return "INFO";
        case 3: return "NOTICE";
        case 4: return "INFO";
        case 5: return "WARN";
        case 6: return "ERROR";
        case 7: return "CRITICAL";
        default: return "LOG(" + std::to_string(level) + ")";
    }
}

// ---------- TCP Server helper ----------
int setupServer(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) throw std::runtime_error("socket() failed");

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");

    if (listen(sock, 1) < 0)
        throw std::runtime_error("listen() failed");

    return sock;
}

// ---------- Command Server Thread ----------
void commandServerThread(int port) {
    int serverSock = -1;
    try {
        serverSock = setupServer(port);
        addLog("[CommandServer] Listening on port " + std::to_string(port));

        while (running) {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            
            int conn = accept(serverSock, (sockaddr*)&clientAddr, &clientLen);
            
            if (conn < 0) {
                if (running) {
                    addLog("[CommandServer] accept() failed: " + std::string(std::strerror(errno)));
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
                continue;
            }

            commandClientSock.store(conn); 
            addLog("[CommandServer] DAQ client connected (FD=" + std::to_string(conn) + ")");

            char buffer[1024];
            while (running) {
                ssize_t n = recv(commandClientSock.load(), buffer, sizeof(buffer)-1, 0);

                if (n > 0) {
                    // Data received
                } else if (n == 0) {
                    addLog("[CommandServer] DAQ disconnected gracefully");
                    break;
                } else {
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        addLog("[CommandServer] Connection error: " + std::string(std::strerror(errno)));
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }

            int fd_to_close = commandClientSock.exchange(-1);
            if (fd_to_close >= 0) {
                ::close(fd_to_close);
            }
            addLog("[CommandServer] Waiting for new DAQ reconnection...");
        }

    } catch (const std::exception& e) {
        addLog(std::string("[CommandServer] Fatal Exception: ") + e.what());
    }

    if (serverSock >= 0) {
        ::close(serverSock);
    }
}

// ---------- Event Server Thread ----------
void eventServerThread(int port) { 
    int serverSock = -1;
    try {
        serverSock = setupServer(port);
        addLog("[EventServer] Listening on port " + std::to_string(port));

        while (running) {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            
            int conn = accept(serverSock, (sockaddr*)&clientAddr, &clientLen);
            if (conn < 0) {
                if (running) std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }

            eventClientSock.store(conn);
            addLog("[EventServer] DAQ client connected (FD=" + std::to_string(conn) + ")");

            std::vector<uint8_t> streamBuffer;
            uint8_t rawBuf[4096];

            while (running) {
                ssize_t n = recv(eventClientSock.load(), rawBuf, sizeof(rawBuf), 0); 
                
                if (n > 0) {
                    streamBuffer.insert(streamBuffer.end(), rawBuf, rawBuf + n);

                    GRAMS_TOF_CommandCodec::Packet respPkt;
                    while (streamBuffer.size() >= 14 && GRAMS_TOF_CommandCodec::parse(streamBuffer, respPkt)) {
                        
                        // 1. Handle Command Callback Response (0x5FFE)
                        if (respPkt.code == static_cast<uint16_t>(TOFCommandCode::CALLBACK) && respPkt.argc >= 2) {
                            uint16_t reportedCmd = static_cast<uint16_t>(respPkt.argv[0]);
                            uint32_t status = static_cast<uint32_t>(respPkt.argv[1]);
                        
                            std::lock_guard<std::mutex> lock(callbackMutex);
                            
                            std::stringstream ss;
                            ss << "[EventServer] Received Callback for 0x" 
                               << std::hex << std::uppercase << reportedCmd 
                               << " Status: " << std::dec << status;
                            addLog(ss.str());
                        
                            if (activeTargetCode != 0 && reportedCmd == activeTargetCode) {
                                lastFinishedCmd = reportedCmd;
                                lastCmdStatus = status;
                                callbackCv.notify_all();
                            }
                        }
                        // 2. Handle Remote Quill Logger Stream Packet (0x5401)
                        else if (respPkt.code == static_cast<uint16_t>(TOFCommandCode::LOGGER_DATA_STREAM)) {
                            GRAMS_TOF_LogCodec::LogData logData;
                            if (GRAMS_TOF_LogCodec::decode(respPkt, logData)) {
                                std::stringstream ss;
                                ss << "[REMOTE-" << getLogLevelString(logData.level) << "] "
                                   << "[" << logData.component << "] "
                                   << logData.message;
                                addLog(ss.str());
                            }
                        }

                        // --- MQTT Forwarding Logic ---
                        // Forward telemetry packet (Histograms, Logs, etc.) directly to MQTT
                        if (g_mosq) {
                            auto wireData = GRAMS_TOF_CommandCodec::serialize(respPkt);
                            int ret = mosquitto_publish(
                                g_mosq,
                                nullptr,
                                "TOF_ground_telemetry",
                                static_cast<int>(wireData.size()),
                                wireData.data(),
                                0,
                                false
                            );
                            if (ret != MOSQ_ERR_SUCCESS) {
                                addLog("[MQTT] Failed to publish packet to broker");
                            }
                        }

                        // Erase parsed packet length from the buffer
                        size_t parsedLen = GRAMS_TOF_CommandCodec::getPacketSize(respPkt);
                        if (parsedLen <= streamBuffer.size()) {
                            streamBuffer.erase(streamBuffer.begin(), streamBuffer.begin() + parsedLen);
                        } else {
                            break;
                        }
                    }

                } else if (n == 0) {
                    addLog("[EventServer] DAQ disconnected gracefully");
                    break; 
                } else {
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        addLog("[EventServer] Connection error: " + std::string(std::strerror(errno)));
                        break; 
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
            }

            int fd_to_close = eventClientSock.exchange(-1);
            if (fd_to_close >= 0) ::close(fd_to_close);
            addLog("[EventServer] Waiting for new DAQ reconnection...");
        }
    } catch (const std::exception& e) {
        addLog(std::string("[EventServer] Fatal Exception: ") + e.what());
    }

    if (serverSock >= 0) ::close(serverSock);
}

// ---------- Python Socket Bridge Thread ----------
void pythonBridgeThread(int port) {
    int serverSock = -1;
    try {
        serverSock = setupServer(port);
        addLog("[PythonBridge] Listening for external commands on port " + std::to_string(port));

        while (running) {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            int conn = accept(serverSock, (sockaddr*)&clientAddr, &clientLen);
            
            if (conn < 0) {
                if (running) std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            char buffer[256] = {0};
            ssize_t n = recv(conn, buffer, sizeof(buffer) - 1, 0);
            
            if (n > 0) {
                std::string inputStr(buffer);
                while (!inputStr.empty() && (inputStr.back() == '\r' || inputStr.back() == '\n' || inputStr.back() == ' ')) {
                    inputStr.pop_back();
                }

                addLog("[PythonBridge] Received external command: " + inputStr);

                if (commandClientSock.load() >= 0) {
                    try {
                        std::vector<int> args;
                        std::vector<std::string> tokens;
                        std::istringstream iss(inputStr);
                        std::string token;
                        
                        while (iss >> token) tokens.push_back(token);

                        if (!tokens.empty()) {
                            uint16_t targetCode = static_cast<uint16_t>(std::stoul(tokens[0], nullptr, 0));
                            for (size_t i = 1; i < tokens.size(); ++i) {
                                args.push_back(std::stoi(tokens[i]));
                            }
                        
                            {
                                std::lock_guard<std::mutex> lock(callbackMutex);
                                activeTargetCode = targetCode;
                                lastFinishedCmd = 0;
                                lastCmdStatus = 0;
                            }
                        
                            auto pkt = buildPacket(targetCode, args);
                            sendPacket(commandClientSock.load(), pkt);
                            logPacketSent(pkt);
                            
                            addLog("[PythonBridge] Command sent. Waiting for 0x5FFE callback...");
                        
                            std::unique_lock<std::mutex> lock(callbackMutex);
                            bool finished = callbackCv.wait_for(lock, std::chrono::seconds(3600), [targetCode]() {
                                return lastFinishedCmd == targetCode;
                            });
                        
                            activeTargetCode = 0;
                        
                            if (finished && lastCmdStatus == 0) {
                                std::string ack = "DONE:" + tokens[0] + "\n";
                                send(conn, ack.c_str(), ack.size(), 0);
                                addLog("[PythonBridge] Command 0x" + tokens[0] + " COMPLETED SUCCESSFUL.");
                            } else if (finished) {
                                std::string err = "ERROR: Command failed with status " + std::to_string(lastCmdStatus) + "\n";
                                send(conn, err.c_str(), err.size(), 0);
                            } else {
                                std::string err = "ERROR: Timeout waiting for callback\n";
                                send(conn, err.c_str(), err.size(), 0);
                            }
                        }
                    } catch (const std::exception& e) {
                        addLog("[PythonBridge] Command parse error: " + std::string(e.what()));
                        std::string err = "ERROR: Parse failure\n";
                        send(conn, err.c_str(), err.size(), 0);
                    }
                } else {
                    addLog("[PythonBridge] REJECTED: No DAQ core client connected!");
                    std::string err = "ERROR: No DAQ client connected\n";
                    send(conn, err.c_str(), err.size(), 0);
                }
            }
            ::close(conn);
        }
    } catch (const std::exception& e) {
        addLog("[PythonBridge] Exception: " + std::string(e.what()));
    }

    if (serverSock >= 0) ::close(serverSock);
}

void heartbeatTask() {
    while (running) {
        int currentSock = commandClientSock.load();
        if (currentSock >= 0) {
            try {
                GRAMS_TOF_CommandCodec::Packet hb;
                hb.code = static_cast<uint16_t>(TOFCommandCode::HEART_BEAT);
                hb.argc = 0;
            
                std::vector<uint8_t> data = GRAMS_TOF_CommandCodec::serialize(hb);
                send(currentSock, data.data(), data.size(), MSG_NOSIGNAL);
            } catch (...) {
                addLog("[Heartbeat] Send failed");
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(heartbeatInterval.load()));
    }
}

// ---------- Main ----------
int main() {
    // Initialize Mosquitto MQTT Library
    mosquitto_lib_init();
    g_mosq = mosquitto_new("DummyHub_Forwarder", true, nullptr);
    if (g_mosq) {
        if (mosquitto_connect(g_mosq, "localhost", 1883, 60) == MOSQ_ERR_SUCCESS) {
            mosquitto_loop_start(g_mosq);
        } else {
            g_mosq = nullptr;
        }
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_CYAN, -1);
    }

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int inputHeight = 3;
    int logHeight = rows - inputHeight;

    WINDOW* logWin = newwin(logHeight, cols, 0, 0);
    WINDOW* inputWin = newwin(inputHeight, cols, logHeight, 0);

    scrollok(logWin, TRUE);
    box(inputWin, 0, 0);
    wrefresh(logWin);
    wrefresh(inputWin);

    std::string inputStr;

    mvwprintw(inputWin, 1, 1, "> ");
    wmove(inputWin, 1, 3);
    wrefresh(inputWin);

    nodelay(inputWin, TRUE);
    keypad(inputWin, TRUE);

    std::thread tCommand(commandServerThread, 50007);
    std::thread tEvent(eventServerThread, 50006);
    std::thread tHeartbeat(heartbeatTask);
    std::thread tPythonBridge(pythonBridgeThread, 50008);

    while (running) {
        {
            std::lock_guard<std::mutex> lock(logMutex);
            while (!logQueue.empty()) {
                std::string msg = logQueue.front();
                logQueue.pop();

                bool isRemoteLog = (msg.rfind("[REMOTE-", 0) == 0);

                if (isRemoteLog && has_colors()) {
                    wattron(logWin, COLOR_PAIR(1) | A_BOLD);
                    wprintw(logWin, "%s\n", msg.c_str());
                    wattroff(logWin, COLOR_PAIR(1) | A_BOLD);
                } else {
                    wprintw(logWin, "%s\n", msg.c_str());
                }
            }
            wrefresh(logWin);
        }

        int ch = wgetch(inputWin);
        if (ch != ERR) {
            if (ch == '\n') {
                if (!inputStr.empty()) {
                    if (inputStr.find("heartbeat ") == 0) {
                        try {
                            int newInterval = std::stoi(inputStr.substr(10));
                            heartbeatInterval = newInterval;
                            addLog("[System] Heartbeat interval set to " + std::to_string(newInterval) + "s");
                        } catch (...) {
                            addLog("[System] Invalid format. Use: heartbeat <seconds>");
                        }
                        inputStr.clear();
                    }

                    if (commandClientSock >= 0) {
                        try {
                            std::vector<int> args;
                            std::vector<std::string> tokens;
                            std::istringstream iss(inputStr);
                            std::string token;
                            while (iss >> token) {
                                tokens.push_back(token);
                            }
                                
                            if (!tokens.empty()) {
                                auto code = std::stoul(tokens[0], nullptr, 0);
                                for (size_t i = 1; i < tokens.size(); ++i) {
                                    args.push_back(std::stoi(tokens[i]));
                                }

                                {
                                     std::lock_guard<std::mutex> lock(callbackMutex);
                                     activeTargetCode = static_cast<uint16_t>(code);
                                }
                               
                                auto pkt = buildPacket(code, args);
                                sendPacket(commandClientSock, pkt);
                                logPacketSent(pkt);
                            }
                        } catch (const std::exception& e) {
                            addLog(std::string("[Command] Failed to build packet: ") + e.what());
                        }
                    } else {
                        addLog("[Command] No client connected!");
                    }
                    inputStr.clear();
                }
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (!inputStr.empty()) inputStr.pop_back();
            } else if (isprint(ch)) {
                inputStr.push_back(ch);
            }
    
            wclear(inputWin);
            box(inputWin, 0, 0);
            mvwprintw(inputWin, 1, 1, "> %s", inputStr.c_str());
        }
    
        wmove(inputWin, 1, 3 + inputStr.size());
        wrefresh(inputWin);
    
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    running = false;
    tCommand.join();
    tEvent.join();
    tHeartbeat.join();
    tPythonBridge.join();

    // Cleanup Mosquitto
    if (g_mosq) {
        mosquitto_loop_stop(g_mosq, true);
        mosquitto_destroy(g_mosq);
    }
    mosquitto_lib_cleanup();

    delwin(logWin);
    delwin(inputWin);
    endwin();

    std::cout << "Exiting test DAQ server." << std::endl;
    return 0;
}
