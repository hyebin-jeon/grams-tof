#include "GRAMS_TOF_DAQController.h"
#include "CLI11.hpp" 
#include "GRAMS_TOF_Logger.h" 
#include "GRAMS_TOF_RuntimeError.h"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {

    CLI::App app{"GRAMS TOF DAQ Core Application (Client Host)"};
    GRAMS_TOF_DAQController::Config config;
    //Logger::instance().setLogLevel(Logger::Level::Detail);
    Logger::instance().setLogLevel(Logger::Level::Info);

    app.add_flag("--no-fpga", config.noFpgaMode, "Skip DAQ initialization for testing without FPGA");
    app.add_option("--command-port", config.commandListenPort, "Command service port (Listening)");
    app.add_option("--event-port", config.eventTargetPort, "Remote Event Server port (Sending)");
    app.add_option("--command-ip", config.remoteCommandHub, "Remote Command Server IP address");
    app.add_option("--event-ip", config.remoteEventHub, "Remote Event Server IP address");
    app.add_option("--config-file", config.configFile, "Path to the config.ini file");
    app.add_option("--log-file", config.logFile, "Path to the DAQ log file");

    try {
        CLI11_PARSE(app, argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    if (config.configFile.empty()) {
        if (!GRAMS_TOF_Config::loadDefaultConfig()) {
            throw GRAMS_TOF_RuntimeError("[System] Configuration file not specified and fallback default GLIB path failed to load.");
        } else config.configFile =  GRAMS_TOF_Config::instance().getConfigFilePath();
    }

    std::unique_ptr<GRAMS_TOF_DAQController> daqController;
    try {
        daqController = std::make_unique<GRAMS_TOF_DAQController>(config);
        if (!daqController->initialize()) {
            Logger::instance().error("[System] DAQ controller initialization loop refused to start. Aborting execution.");
            return 1;
        }

        Logger::instance().info("[System] TOF DAQ Online. Press [Enter] at any point to trigger a clean system shutdown.");

        std::thread runThread([&](){ daqController->run(); });
        std::cin.get(); // Blocking wait for user input to trigger shutdown
        
        // 4. Shutdown
        Logger::instance().info("[System] Shutdown request verified. Stopping engine loops...");
        daqController->stop();
        if (runThread.joinable()) {
            runThread.join();
        }
        Logger::instance().info("[System] DAQ application closed cleanly.");
        return 0;
    } 
    catch (const GRAMS_TOF_RuntimeError& e) {
        Logger::instance().error("[System] Core Panic: Exception caught at process boundary: {}", e.what());
        return 1;
    }
    catch (const std::exception& e) {
        Logger::instance().error("[System] Standard Library Exception caught at process boundary: {}", e.what());
        return 1;
    } catch (...) {
        Logger::instance().error("[System] Unknown structural exception intercepted at process boundary.");
        return 1;
    }
}

