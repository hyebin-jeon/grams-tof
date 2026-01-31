#include "GRAMS_TOF_DAQController.h"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <cstring>

namespace {
    void ensureStandardFDs() {
        for (int fd = 0; fd <= 2; ++fd) {
            if (fcntl(fd, F_GETFD) == -1 && errno == EBADF) {
                int newfd = open("/dev/null", O_RDWR);
                if (newfd != fd) {
                    dup2(newfd, fd);
                    close(newfd);
                }
            }
        }
    }
}

// Helper to initialize logging and configuration
void GRAMS_TOF_DAQController::setupSystemFiles() {
    ensureStandardFDs();
    signal(SIGPIPE, SIG_IGN);
    Logger::instance().setLogFile(config_.logFile);
    try {
        GRAMS_TOF_Config::instance().setConfigFile(config_.configFile);
    } catch (const std::exception& e) {
        Logger::instance().error("[System] Config load error: {}", e.what());
        throw; // Re-throw to indicate fatal configuration error
    }
}

// Constructor Implementation
GRAMS_TOF_DAQController::GRAMS_TOF_DAQController(const Config& config)
    : config_(config),
      daq_("/tmp/d.sock", "/daqd_shm", 0, "GBE", {"/dev/psdaq0"}),
      pyint_(daq_),
      eventClient_(std::make_unique<GRAMS_TOF_EventClient>(
          config.remoteEventHub, 
          config.eventTargetPort,
          [](const GRAMS_TOF_CommandCodec::Packet& pkt) {
          }
      )),
      commandClient_(std::make_unique<GRAMS_TOF_CommandClient>(
          config.remoteCommandHub,
          config.commandListenPort,
          [this](const GRAMS_TOF_CommandCodec::Packet& pkt) {
              this->handleIncomingCommand(pkt);
          }
      )),
      dispatchTable_(pyint_, analyzer_, *eventClient_)
{
    setupSystemFiles();
    setenv("DEBUG", "1", 1);
}

// Initialization Implementation (Hardware Setup)
bool GRAMS_TOF_DAQController::initialize() {
    Logger::instance().info("[System] Initializing DAQ with Config: IP={}, CP={}, EP={}, NoFPGA={}",
                            config_.remoteEventHub, config_.commandListenPort,
                            config_.eventTargetPort, config_.noFpgaMode);

    if (!config_.noFpgaMode) {
        if (!daq_.initialize()) {
            Logger::instance().error("[System] DAQ initialization failed.");
            return false;
        }
    } else {
        Logger::instance().info("[System] Running in no-FPGA mode (DAQ init skipped)");
    }

    // Start network services
    eventClient_->start();
    commandClient_->start();

    Logger::instance().info("[System] Waiting for Hub connection...");

    int retries = 0;
    const int max_retries = 50; // 5 seconds
    while (keepRunning_ && retries < max_retries) {
        bool eventReady = eventClient_->isConnected();
        bool commandReady = commandClient_->isConnected();

        if (eventReady && commandReady) {
            Logger::instance().info("[System] All Hub connections established.");
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        retries++;
    }

    if (retries >= max_retries) {
        Logger::instance().warn("[System] Startup synchronization timed out. Network may be degraded.");
    }

    Logger::instance().info("[System] Event service (CLIENT) sending to {}:{}", config_.remoteEventHub, config_.eventTargetPort);
    Logger::instance().info("[System] Command service (SERVER) listening on port {}", config_.commandListenPort);

    return true;
}

void GRAMS_TOF_DAQController::handleIncomingCommand(const GRAMS_TOF_CommandCodec::Packet& pkt) {
    TOFCommandCode code = tof_bridge::toTOFCommand(
        static_cast<pgrams::communication::CommunicationCodes>(pkt.code)
    );

    const auto& argv = pkt.argv;
    if (!dispatchTable_.dispatch(code, argv)) {
        Logger::instance().error("[CommandClient] Command failed or unknown: 0x{:04X}", static_cast<int>(code));
    }
}

void GRAMS_TOF_DAQController::run() {
    Logger::instance().info("[System] GRAMS_TOF_DAQController running...");

    while (keepRunning_) {
        // Sleep briefly to avoid busy-waiting and allow other threads to run
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    Logger::instance().info("[System] Finalizing network transmissions...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    commandClient_->stop();
    eventClient_->stop();

    Logger::instance().info("[System] GRAMS_TOF_DAQController exited run loop and shut down services.");
}

void GRAMS_TOF_DAQController::stop() {
    Logger::instance().info("[System] Received STOP signal. Shutting down TOF DAQ...");
    keepRunning_ = false;
}
