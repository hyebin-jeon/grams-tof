#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <set>
#include <csignal>
#include <sys/wait.h>

#include "GRAMS_TOF_DAQManager.h"
#include "GRAMS_TOF_CommandCodec.h"
#include "GRAMS_TOF_PythonIntegration.h"
#include "GRAMS_TOF_Analyzer.h"
#include "GRAMS_TOF_EventClient.h"

class GRAMS_TOF_CommandDispatch {
public:
    GRAMS_TOF_CommandDispatch(GRAMS_TOF_PythonIntegration& pyint, 
                              GRAMS_TOF_Analyzer& analyzer,
                              GRAMS_TOF_EventClient& eventClient);
    ~GRAMS_TOF_CommandDispatch();

    using CommandArgs = std::vector<uint32_t>;
    using CommandHandler = std::function<bool(const CommandArgs&)>;
    bool dispatch(TOFCommandCode code, const CommandArgs& argv);

private:
    GRAMS_TOF_PythonIntegration& pyint_;
    GRAMS_TOF_Analyzer& analyzer_;
    GRAMS_TOF_EventClient& eventClient_;

    std::unordered_map<TOFCommandCode, CommandHandler> table_;

    // Thread and control for DAQ run
    std::thread daqThread_;
    std::atomic<bool> daqRunning_{false};
    std::mutex daqMutex_;

    // Managed Background Process Tracking
    std::map<pid_t, TOFCommandCode> activeBackgroundPIDs_;
    std::mutex pidMutex_;

    // Helper to launch and track background scripts
    bool executeManagedBackground(TOFCommandCode code, const std::string& scriptName, const std::vector<std::string>& args, const std::string& interpreter = "python3");

    // Monitor Thread to watch background PIDs
    std::thread monitorThread_;
    std::atomic<bool> monitorRunning_{false};
    void runMonitorThread();

    // Send Callback
    void sendStatusCallback(TOFCommandCode code, int32_t status);
    bool executeSimpleCommand(TOFCommandCode code, std::function<bool()> func);

    // Thread function to run DAQ
    void runDAQThread();
};

