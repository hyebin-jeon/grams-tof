#pragma once

#include "GRAMS_TOF_CommandCodec.h"
#include "GRAMS_TOF_Client.h"
#include "GRAMS_TOF_FDManager.h"

#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <queue> 
#include <condition_variable>

class GRAMS_TOF_CommandClient {
public:
    using CommandHandler = std::function<void(const GRAMS_TOF_CommandCodec::Packet&)>;

    // Client constructor takes IP and Port
    explicit GRAMS_TOF_CommandClient(const std::string& hub_ip, int port, CommandHandler handler);
    ~GRAMS_TOF_CommandClient();

    void start();
    void stop();

    GRAMS_TOF_CommandClient(const GRAMS_TOF_CommandClient&) = delete;
    GRAMS_TOF_CommandClient& operator=(const GRAMS_TOF_CommandClient&) = delete;

    bool isConnected() const;
    bool isHealthy() const;

private:
    void run();  // Main client connection/read loop
    void workerLoop(); // Background worker function

    // Configuration
    std::string hub_ip_;
    int port_;
    CommandHandler handler_;

    // Threading and execution control
    std::thread client_thread_;
    std::thread worker_thread_;
    std::atomic<bool> running_{false};

    // Async Command Queue
    std::deque<GRAMS_TOF_CommandCodec::Packet> commandQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;

    // Connection State (Replaces the server's clientList_)
    std::unique_ptr<GRAMS_TOF_Client> hubConnection_;
    mutable std::mutex connectionMutex_; // Protects access to hubConnection_

    // ACK healty checker
    std::chrono::steady_clock::time_point last_activity_time_;
    const std::chrono::seconds heartbeat_timeout_{5}; //default 5 sec
};
