#include "GRAMS_TOF_CommandDispatch.h"
#include "GRAMS_TOF_Logger.h"
#include "GRAMS_TOF_Config.h"
#include "GRAMS_TOF_RootConverter.h"
#include <unistd.h>
#include <sys/wait.h>
#include <filesystem>

GRAMS_TOF_CommandDispatch::GRAMS_TOF_CommandDispatch(
    GRAMS_TOF_PythonIntegration& pyint,
    GRAMS_TOF_Analyzer& analyzer,
    GRAMS_TOF_EventClient& eventClient,
    GRAMS_TOF_SystemEventListener& listener)
    : pyint_(pyint), analyzer_(analyzer), eventClient_(eventClient), listener_(listener), table_{}
{
    auto& config = GRAMS_TOF_Config::instance();

    // START_DAQ
    table_[TOFCommandCode::START_DAQ] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::START_DAQ, [&]() {
            Logger::instance().info("[CommandDispatch][START] Starting DAQ...");
            std::lock_guard<std::mutex> lock(daqMutex_);
    
            if (daqRunning_) return false;
    
            if (daqThread_.joinable()) {
                daqThread_.join();
            }
    
            if (!pyint_.getDAQ().initialize()) {
                Logger::instance().error("[CommandDispatch][START] Failed to re-initialize DAQ Manager");
                return false;
            }
   
            daqRunning_ = true;
            daqThread_ = std::thread(&GRAMS_TOF_CommandDispatch::runDAQThread, this);
            return true; 
        });
    };

    // STOP_DAQ
    table_[TOFCommandCode::STOP_DAQ] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::STOP_DAQ, [&]() {
            Logger::instance().warn("[CommandDispatch][STOP] Initiating Master Stop Sequence...");
   
            // 0. Cancel macro loops
            macroLoopRunning_ = false;
 
            // 1. HARDWARE STOP (Non-blocking async process)
            std::string stopScriptPath = pyint_.resolveScriptPath("stop_daq.py");
            pid_t stop_pid = fork();
            if (stop_pid == 0) {
                for (int i = 3; i < 1024; ++i) ::close(i);
                execlp("python3", "python3", stopScriptPath.c_str(), nullptr);
                _exit(1);
            } else if (stop_pid > 0) {
                // Register with monitor thread so it reaps without blocking or setting SIG_IGN
                {
                    std::lock_guard<std::mutex> lock(pidMutex_);
                    activeBackgroundTasks_[stop_pid] = BackgroundTask{TOFCommandCode::STOP_DAQ, nullptr};
                }
                Logger::instance().info("[CommandDispatch][STOP] Hardware stop script triggered (PID: {}).", stop_pid);
            }
   
            // 2. INTERRUPT MANAGED BACKGROUND PROCESSES
            {
                std::lock_guard<std::mutex> lock(pidMutex_);
                for (auto const& [b_pid, task] : activeBackgroundTasks_) {
                    if (b_pid != stop_pid) { 
                        Logger::instance().info("[CommandDispatch][STOP] Terminating {} (PID: {})", static_cast<int>(task.commandCode), b_pid);
                        kill(b_pid, SIGTERM);
                        sendStatusCallback(task.commandCode, 2); 
                    }
                }
                activeBackgroundTasks_.clear();
                if (stop_pid > 0) {
                    activeBackgroundTasks_[stop_pid] = BackgroundTask{TOFCommandCode::STOP_DAQ, nullptr};
                }
            }

            // 3. SIGNAL SOFTWARE STOP
            pyint_.getDAQ().stop();
    
            // 4. JOIN THREAD 
            if (daqThread_.joinable() && std::this_thread::get_id() != daqThread_.get_id()) {
                daqThread_.join();
                Logger::instance().debug("[CommandDispatch][STOP] DAQ thread joined.");
            }
    
            {
                std::lock_guard<std::mutex> lock(daqMutex_);
                daqRunning_ = false;
            }
    
            Logger::instance().info("[CommandDispatch][STOP] Full system interrupt complete.");
            return true;
        });
    };

    // RESET_DAQ
    table_[TOFCommandCode::RESET_DAQ] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RESET_DAQ, [&]() {
            Logger::instance().info("[CommandDispatch][RESET] Initiating Reset (STOP -> START pipeline)...");
    
            // 1. Invoke the core logic of STOP_DAQ inline 
            if (table_.count(TOFCommandCode::STOP_DAQ)) {
                Logger::instance().info("[CommandDispatch][RESET] Step 1: Executing STOP sequence...");
                table_[TOFCommandCode::STOP_DAQ](argv); 
            }
    
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
            // 2. Invoke the core logic of START_DAQ inline
            if (table_.count(TOFCommandCode::START_DAQ)) {
                Logger::instance().info("[CommandDispatch][RESET] Step 2: Executing START sequence...");
                bool start_success = table_[TOFCommandCode::START_DAQ](argv);
                
                if (!start_success) {
                    Logger::instance().error("[CommandDispatch][RESET] Pipeline failed: START_DAQ rejected initialization.");
                    return false;
                }
            }
    
            Logger::instance().info("[CommandDispatch][RESET] System successfully cycled and restarted.");
            return true;
        });
    };

    // RECONNECT_NETWORK
    table_[TOFCommandCode::RECONNECT_NETWORK] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        Logger::instance().info("[CommandDispatch] Manual network reconnection requested...");
        listener_.onNetworkResetRequested(); 
        return true;
    };

    // INIT_SYSTEM
    table_[TOFCommandCode::INIT_SYSTEM] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::INIT_SYSTEM, [&]() {
            Logger::instance().info("[CommandDispatch] Executing init_system.py script...");
            return pyint_.runPetsysInitSystem("scripts.init_system");
        });
    };

    // MAKE_BIAS_CALIB_TABLE
    table_[TOFCommandCode::MAKE_BIAS_CALIB_TABLE] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::MAKE_BIAS_CALIB_TABLE, [&]() {
            Logger::instance().info("[CommandDispatch] Executing make_bias_calibration_table.py script...");
            return pyint_.runPetsysMakeBiasCalibrationTable(
                "scripts.make_bias_calibration_table",
                config.getString("main", "bias_calibration_table"),
                {argv.size() > 0 ? static_cast<int>(argv[0]) : 0},
                {argv.size() > 1 ? static_cast<int>(argv[1]) : 0},
                {argv.size() > 2 ? static_cast<int>(argv[2]) : 0},
                {}
            );
        });
    };

    // MAKE_SIMPLE_BIAS_SET_TABLE
    table_[TOFCommandCode::MAKE_SIMPLE_BIAS_SET_TABLE] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::MAKE_SIMPLE_BIAS_SET_TABLE, [&]() {
            Logger::instance().info("[CommandDispatch] Executing make_simple_bias_settings_table.py script...");
            return pyint_.runPetsysMakeSimpleBiasSettingsTable(
                "scripts.make_simple_bias_settings_table",
                config.getConfigFilePath(),
                argv.size() > 0 ? static_cast<float>(argv[0]) : 0.75f,
                argv.size() > 1 ? static_cast<float>(argv[1]) : 20.0f,
                argv.size() > 2 ? static_cast<float>(argv[2]) : 24.9f,
                argv.size() > 3 ? static_cast<float>(argv[3]) : 5.0f,
                config.getString("main", "bias_settings_table")
            );
        });
    };

    // MAKE_SIMPLE_CHANNEL_MAP
    table_[TOFCommandCode::MAKE_SIMPLE_CHANNEL_MAP] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::MAKE_SIMPLE_CHANNEL_MAP, [&]() {
            Logger::instance().info("[CommandDispatch] Executing make_simple_channel_map.py script...");
            return pyint_.runPetsysMakeSimpleChannelMap(
                "scripts.make_simple_channel_map",
                config.getConfigDir() + "/map"
            );
        });
    };

    // MAKE_SIMPLE_DISC_SET_TABLE
    table_[TOFCommandCode::MAKE_SIMPLE_DISC_SET_TABLE] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::MAKE_SIMPLE_DISC_SET_TABLE, [&]() {
            Logger::instance().info("[CommandDispatch] Executing make_simple_disc_settings_table.py script...");
            return pyint_.runPetsysMakeSimpleDiscSettingsTable(
                "scripts.make_simple_disc_settings_table",
                config.getConfigFilePath(),
                argv.size() > 0 ? argv[0] : 20,
                argv.size() > 1 ? argv[1] : 20,
                argv.size() > 2 ? argv[2] : 15,
                config.getString("main", "disc_settings_table")
            );
        });
    };

    // READ_TEMPERATURE_SENSORS
    table_[TOFCommandCode::READ_TEMPERATURE_SENSORS] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().info("[CommandDispatch] Executing read_temperature_sensors.py script...");
            std::vector<std::string> sArgs;
            sArgs.push_back("--time");
            sArgs.push_back(std::to_string(argv.size() > 0 ? static_cast<double>(argv[0]) : 0.0));
            sArgs.push_back("--interval");
            sArgs.push_back(std::to_string(argv.size() > 1 ? static_cast<double>(argv[1]) : 60.0));
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getLogDir(), "read_temperature_sensors", timestampStr, "log"));
            bool startup = (argv.size() > 2) ? (argv[2] != 0) : true;
            if (startup) sArgs.push_back("--startup");
            bool debug = (argv.size() > 3) ? (argv[3] != 0) : false;
            if (debug) sArgs.push_back("--debug");

            return executeManagedBackground(TOFCommandCode::READ_TEMPERATURE_SENSORS, "read_temperature_sensors.py", sArgs);
        } catch (...) {
            Logger::instance().error("[CommandDispatch] Exception in READ_TEMPERATURE_SENSORS");
            return false;
        }
    };

    // ACQUIRE_THRESHOLD_CALIBRATION
    table_[TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().info("[CommandDispatch] Starting full threshold calibration in background...");
            
            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getCalibrationDir(), "disc_calibration", timestampStr));
            sArgs.push_back("--mode");
            sArgs.push_back("all");
            sArgs.push_back("--nreads-noise");
            sArgs.push_back(std::to_string(argv.size() > 0 ? argv[0] : 4));
            sArgs.push_back("--nreads-dark");
            sArgs.push_back(std::to_string(argv.size() > 1 ? argv[1] : 4));
            bool bias = (argv.size() > 2) ? (argv[2] != 0) : false;
            if (bias) sArgs.push_back("--ext-bias");

            return executeManagedBackground(TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION, "acquire_threshold_calibration.py", sArgs);
        } catch (...) {
            Logger::instance().error("[CommandDispatch] Exception in ACQUIRE_THRESHOLD_CALIBRATION");
            return false;
        }
    };

    // ACQUIRE_THRESHOLD_CALIBRATION_BN (Baseline and Noise only) ---
    table_[TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_BN] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().info("[CommandDispatch] Starting Baseline/Noise calibration in background...");
    
            std::string outPathPrefix = config.makeFilePathWithTimestamp(config.getCalibrationDir(), "disc_calibration", timestampStr);
    
            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(outPathPrefix);
            sArgs.push_back("--mode");
            sArgs.push_back("baseline_noise");
            sArgs.push_back("--nreads-noise");
            sArgs.push_back(std::to_string(argv.size() > 0 ? argv[0] : 4));
            bool bias = (argv.size() > 2) ? (argv[2] != 0) : false;
            if (bias) sArgs.push_back("--ext-bias");
    
            std::string generatedBaseline = outPathPrefix + "_baseline.tsv";
            std::string stableConfigTarget = config.getConfigDir() + "/disc_calibration_baseline.tsv";
    
            auto callback = [generatedBaseline, stableConfigTarget](bool processSuccess) {
                if (processSuccess) {
                    try {
                        auto& cfg = GRAMS_TOF_Config::instance();
                        cfg.copyOrLink(generatedBaseline, stableConfigTarget, true);
                        Logger::instance().info("[CommandDispatch] Centralized baseline symlink updated: {}", stableConfigTarget);
                    } catch (const std::exception& e) {
                        Logger::instance().error("[CommandDispatch] Symlink post-processing failed: {}", e.what());
                    }
                }
            };
    
            return executeManagedBackground(
                TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_BN, 
                "acquire_threshold_calibration.py", 
                sArgs, 
                callback
            );
        } catch (...) {
            Logger::instance().error("[CommandDispatch] Exception in ACQUIRE_THRESHOLD_CALIBRATION_BN");
            return false;
        }
    };

    // ACQUIRE_THRESHOLD_CALIBRATION_D (Dark counts only) ---
    table_[TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_D] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            Logger::instance().info("[CommandDispatch] Starting Dark counts calibration in background...");
    
            // 1. Generate new timestamp for dark count output files
            std::string currentTs = config.getCurrentTimestamp();
    
            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            // Output prefix inside current vault dir (e.g. disc_calibration_2026-07-21_18-59-35.432Z)
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getCalibrationDir(), "disc_calibration", currentTs));
            sArgs.push_back("--mode");
            sArgs.push_back("dark");
            sArgs.push_back("--nreads-dark");
            sArgs.push_back(std::to_string(argv.size() > 0 ? argv[0] : 4));
            
            bool bias = (argv.size() > 2) ? (argv[2] != 0) : false;
            if (bias) sArgs.push_back("--ext-bias");
    
            return executeManagedBackground(TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_D, "acquire_threshold_calibration.py", sArgs);
        } catch (const std::exception& e) {
            Logger::instance().error("[CommandDispatch] Exception in ACQUIRE_THRESHOLD_CALIBRATION_D: {}", e.what());
            return false;
        }
    };

    // ACQUIRE_QDC_CALIBRATION
    table_[TOFCommandCode::ACQUIRE_QDC_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().info("[CommandDispatch] Starting QDC calibration in background...");

            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getCalibrationDir(), "qdc_calibration", timestampStr));

            return executeManagedBackground(TOFCommandCode::ACQUIRE_QDC_CALIBRATION, "acquire_qdc_calibration.py", sArgs);
        } catch (...) {
            Logger::instance().error("[CommandDispatch] Exception in ACQUIRE_QDC_CALIBRATION");
            return false;
        }
    };

    // ACQUIRE_TDC_CALIBRATION
    table_[TOFCommandCode::ACQUIRE_TDC_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().info("[CommandDispatch] Starting TDC calibration in background...");

            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getCalibrationDir(), "tdc_calibration", timestampStr));

            return executeManagedBackground(TOFCommandCode::ACQUIRE_TDC_CALIBRATION, "acquire_tdc_calibration.py", sArgs);
        } catch (...) {
            Logger::instance().error("[CommandDispatch] Exception in ACQUIRE_TDC_CALIBRATION");
            return false;
        }
    };

    // ACQUIRE_SIPM_DATA
    table_[TOFCommandCode::ACQUIRE_SIPM_DATA] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().info("[CommandDispatch] Starting SiPM data acquisition in background...");

            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getSTG0Dir(), "run", timestampStr));
            sArgs.push_back("--time");
            sArgs.push_back(std::to_string(argv.size() > 0 ? static_cast<double>(argv[0]) : sipmDataAcquisitionTime_.load()));
            sArgs.push_back("--mode");
            sArgs.push_back("qdc"); // Defaulting to QDC mode as per your previous logic
            bool hw_trig = (argv.size() > 1) ? (argv[1] != 0) : false;
            if (hw_trig) sArgs.push_back("--enable-hw-trigger");
            
            return executeManagedBackground(TOFCommandCode::ACQUIRE_SIPM_DATA, "acquire_sipm_data.py", sArgs);
        } catch (...) {
            Logger::instance().error("[CommandDispatch] Exception in ACQUIRE_SIPM_DATA");
            return false;
        }
    };

    // SET_FEM_POWER_OFF 
    table_[TOFCommandCode::SET_FEM_POWER_OFF] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::SET_FEM_POWER_OFF, [&]() {
            Logger::instance().info("[CommandDispatch] Executing toggle_system_power.py script...");
            return pyint_.runPetsysToggleSystemPower("scripts.toggle_system_power", "off");
        });
    };

    // SET_FEM_POWER_ON 
    table_[TOFCommandCode::SET_FEM_POWER_ON] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::SET_FEM_POWER_ON, [&]() {
            Logger::instance().info("[CommandDispatch] Executing toggle_system_power.py script...");
            return pyint_.runPetsysToggleSystemPower("scripts.toggle_system_power", "on");
        });
    };

    // PROCESS_THRESHOLD_CALIBRATION
    table_[TOFCommandCode::PROCESS_THRESHOLD_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::PROCESS_THRESHOLD_CALIBRATION, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getCalibrationDir(), "disc_calibration", "_noise.tsv");
            Logger::instance().info("[CommandDispatch] Running threshold calibration...");
            auto output = analyzer_.runPetsysProcessThresholdCalibration(
                config.getConfigFilePath(),
                config.makeFilePathWithTimestamp(config.getCalibrationDir(), "disc_calibration", timestampStr),
                config.makeFilePathWithTimestamp(config.getDiscDir(), "disc_calibration", timestampStr, "tsv"),
                config.makeFilePathWithTimestamp(config.getCalibrationDir(), "disc_calibration", timestampStr, "root")
            );
            config.copyOrLink(config.getFileByTimestamp(config.getDiscDir(), "disc_calibration", timestampStr, ".tsv", true),
                              config.getAbsolutePath("main", "disc_calibration_table"), true);
            return output;
        });
    };

    // PROCESS_TDC_CALIBRATION
    table_[TOFCommandCode::PROCESS_TDC_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::PROCESS_TDC_CALIBRATION, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getCalibrationDir(), "tdc_calibration");
            Logger::instance().info("[CommandDispatch] Running TDC calibration...");
            bool doSorting = argv.size() > 0 ? (argv[0] != 0) : true;
            bool keepTmp   = argv.size() > 1 ? (argv[1] != 0) : false;
            float nominalM = argv.size() > 2 ? static_cast<float>(argv[2]) : 200.0f;
            auto output = analyzer_.runPetsysProcessTdcCalibration(
                config.getConfigFilePath(),
                config.makeFilePathWithTimestamp(config.getCalibrationDir(), "tdc_calibration", timestampStr),
                config.makeFilePathWithTimestamp(config.getTDCDir(), "tdc_calibration", timestampStr),
                config.makeFilePathWithTimestamp(config.getCalibrationDir(), "tdc_calibration", timestampStr),
                doSorting,
                keepTmp,
                nominalM
            );
            config.copyOrLink(config.getFileByTimestamp(config.getTDCDir(), "tdc_calibration", timestampStr, "tsv"),
                              config.getAbsolutePath("main", "tdc_calibration_table"), true);
            return output;
        });
    };

    // PROCESS_QDC_CALIBRATION
    table_[TOFCommandCode::PROCESS_QDC_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::PROCESS_QDC_CALIBRATION, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getCalibrationDir(), "qdc_calibration");
            Logger::instance().info("[CommandDispatch] Running QDC calibration...");
            bool doSorting = argv.size() > 0 ? (argv[0] != 0) : true;
            bool keepTmp   = argv.size() > 1 ? (argv[1] != 0) : false;
            float nominalM = argv.size() > 2 ? static_cast<float>(argv[2]) : 200.0f;
            auto output = analyzer_.runPetsysProcessQdcCalibration(
                config.getConfigFilePath(),
                config.makeFilePathWithTimestamp(config.getCalibrationDir(), "qdc_calibration", timestampStr),
                config.makeFilePathWithTimestamp(config.getQDCDir(), "qdc_calibration", timestampStr),
                config.makeFilePathWithTimestamp(config.getCalibrationDir(), "qdc_calibration", timestampStr),
                doSorting,
                keepTmp,
                nominalM
            );
            config.copyOrLink(config.getFileByTimestamp(config.getQDCDir(), "qdc_calibration", timestampStr, "tsv"),
                              config.getAbsolutePath("main", "qdc_calibration_table"), true);
            return output;
        });
    };


    // CONVERT_RAW_TO_RAW
    table_[TOFCommandCode::CONVERT_RAW_TO_RAW] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::CONVERT_RAW_TO_RAW, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getSTG0Dir(), "run");
            Logger::instance().info("[CommandDispatch] Converting raw to raw...");
            long long eventFractionToWrite = argv.size() > 1 ? static_cast<long long>(argv[0]) : 1024;

            return analyzer_.runPetsysConvertRawToRaw(
                config.getConfigFilePath(),
                config.makeFilePathWithTimestamp(config.getSTG0Dir(), "run", timestampStr),
                config.makeFilePathWithTimestamp(config.getSTG1Dir(), "run", timestampStr, "stg1.root"),
                eventFractionToWrite
            );
        });
    };

    // CONVERT_RAW_TO_SINGLES
    table_[TOFCommandCode::CONVERT_RAW_TO_SINGLES] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::CONVERT_RAW_TO_SINGLES, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getSTG0Dir(), "run");
            Logger::instance().info("[CommandDispatch] Converting raw to singles...");
            PETSYS::FILE_TYPE fileType = argv.size() > 0 ? static_cast<PETSYS::FILE_TYPE>(argv[0]) : PETSYS::FILE_ROOT;
            long long eventFractionToWrite = argv.size() > 1 ? static_cast<long long>(argv[1]) : 1024;
            double fileSplitTime = argv.size() > 2 ? static_cast<double>(argv[2]) : 0.0;

            return analyzer_.runPetsysConvertRawToSingles(
                config.getConfigFilePath(),
                config.getFileByTimestamp(config.getSTG0Dir(), "run", timestampStr),
                config.makeFilePathWithTimestamp(config.getSTG1Dir(), "run", timestampStr, "root_singles"),
                fileType,
                eventFractionToWrite,
                fileSplitTime
            );
        });
    };

    // CONVERT_STG1_TO_STG2
    table_[TOFCommandCode::CONVERT_STG1_TO_STG2] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::CONVERT_STG1_TO_STG2, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getSTG1Dir(), "run");
            Logger::instance().info("[CommandDispatch] Converting stg1 to stg2...");
            return analyzer_.runPetsysConvertStg1ToStg2(
								config.getFileByTimestamp(config.getSTG1Dir(), "run", timestampStr, "stg1.root"),
                config.getSTG2Dir()
            );
        });
    };

    // PROCESS_QA_COIN
    table_[TOFCommandCode::PROCESS_QA_COIN] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::PROCESS_QA_COIN, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getSTG2Dir(), "run");
            Logger::instance().info("[CommandDispatch] Running TOF coin evt calculation...");
            return analyzer_.runPetsysProcessTofCoinEvtQA(
                config.getFileByTimestamp(config.getSTG2Dir(), "run", timestampStr, "stg2.root"),
								config.getSTG2Dir(),
                config.getString("main", "tdc_calibration_table"),
                config.getString("main", "qdc_calibration_table"),
                {argv.size() > 0 ? static_cast<int>(argv[0]) : -1}
            );
        });
    };

    // PROCESS_TOF_QA_IRIDIUM
    table_[TOFCommandCode::PROCESS_QA_IRIDIUM] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::PROCESS_QA_IRIDIUM, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getSTG2Dir(), "run");
            Logger::instance().info("[CommandDispatch] Running TOF Quality Assurance for Iridium...");
            bool output = analyzer_.runPetsysProcessTofQAIridium(
                config.getFileByTimestamp(config.getSTG2Dir(), "run", timestampStr, "stg2.root"),
								config.getHistDir(),
								config.getString("main", "active_asic_list") 
            );

            if (!output) {
                Logger::instance().error("[CommandDispatch] TOF Quality Assurance for Iridium is failed.");
                return false;
            } 

            auto path = config.getFileByTimestamp(config.getHistDir(), "run", timestampStr, "iridiumQA.root");
            auto monitorDataList = GRAMS_TOF_RootConverter::scanFile(path, 0);

            if (monitorDataList.empty()) {
                Logger::instance().error("[CommandDispatch] No histograms found or file missing: {}", path);
                return false;
            }

            for (const auto& data : monitorDataList) {
                eventClient_.sendMonitorData(TOFCommandCode::MONITOR_DATA_STREAM, data); 
            }

            Logger::instance().info("[CommandDispatch] Successfully streamed {} histograms.", monitorDataList.size());
            return true;
        });
    };

    //  MACRO_THERMAL_CALIB
    table_[TOFCommandCode::MACRO_THERMAL_CALIB] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::MACRO_THERMAL_CALIB, [&]() {
            if (macroLoopRunning_) {
                Logger::instance().warn("[CommandDispatch] Another macro loop is already active!");
                return false;
            }
            if (macroLoopThread_.joinable()) {
                macroLoopThread_.join();
            }

            macroLoopRunning_ = true; 
            macroLoopThread_ = std::thread([this]() {
                this->executeMacroSequence("ThermalCalib", "thermal_calib", {
                    TOFCommandCode::READ_TEMPERATURE_SENSORS,
                    TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_BN,
                    TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_D,
                    TOFCommandCode::PROCESS_THRESHOLD_CALIBRATION,
                    TOFCommandCode::MAKE_SIMPLE_DISC_SET_TABLE,
                    TOFCommandCode::ACQUIRE_TDC_CALIBRATION,
                    TOFCommandCode::ACQUIRE_QDC_CALIBRATION,
                    TOFCommandCode::PROCESS_TDC_CALIBRATION,
                    TOFCommandCode::PROCESS_QDC_CALIBRATION
                });
                macroLoopRunning_ = false;
            });
            return true;
        });
    };
    
    //  MACRO_AUTO_RUN_SEQUENCE
    table_[TOFCommandCode::MACRO_AUTO_RUN_SEQUENCE] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::MACRO_AUTO_RUN_SEQUENCE, [&]() {
            if (macroLoopRunning_) {
                Logger::instance().warn("[CommandDispatch] Another macro loop is already active!");
                return false;
            }
            if (macroLoopThread_.joinable()) {
                macroLoopThread_.join();
            }

            macroLoopRunning_ = true; 
            macroLoopThread_ = std::thread([this]() {
                this->executeMacroSequence("AutoRun", "auto_run", {
                    TOFCommandCode::RESET_DAQ,
                    TOFCommandCode::READ_TEMPERATURE_SENSORS,
                    TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_BN,
                    TOFCommandCode::PROCESS_THRESHOLD_CALIBRATION,
                    TOFCommandCode::MAKE_SIMPLE_DISC_SET_TABLE,
                    TOFCommandCode::ACQUIRE_SIPM_DATA,
                    TOFCommandCode::CONVERT_RAW_TO_RAW,
                    TOFCommandCode::CONVERT_STG1_TO_STG2,
                    TOFCommandCode::PROCESS_QA_COIN,
                    TOFCommandCode::PROCESS_QA_IRIDIUM
                });
                macroLoopRunning_ = false;
            });
            return true;
        });
    };
    
    //  MACRO_PRE_BIAS_PREP 
    table_[TOFCommandCode::MACRO_PRE_BIAS_PREP] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::MACRO_PRE_BIAS_PREP, [&]() {
            if (macroLoopRunning_) {
                Logger::instance().warn("[CommandDispatch] Another macro loop is already active!");
                return false;
            }
            if (macroLoopThread_.joinable()) {
                macroLoopThread_.join();
            }

            macroLoopRunning_ = true; 
            macroLoopThread_ = std::thread([this]() {
                this->executeMacroSequence("PreBiasPrep", "pre_bias_prep", {
                    TOFCommandCode::RESET_DAQ,
                    TOFCommandCode::READ_TEMPERATURE_SENSORS,
                    TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_BN,
                    TOFCommandCode::PROCESS_THRESHOLD_CALIBRATION,
                    TOFCommandCode::MAKE_SIMPLE_DISC_SET_TABLE
                });
                macroLoopRunning_ = false;
            });
            return true;
        });
    };

    //  MACRO_POST_BIAS_PREP 
    table_[TOFCommandCode::MACRO_POST_BIAS_PREP] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::MACRO_POST_BIAS_PREP, [&]() {
            if (macroLoopRunning_) {
                Logger::instance().warn("[CommandDispatch] Another macro loop is already active!");
                return false;
            }
            if (macroLoopThread_.joinable()) {
                macroLoopThread_.join();
            }

            macroLoopRunning_ = true; 
            macroLoopThread_ = std::thread([this]() {
                this->executeMacroSequence("PostBiasPrep", "post_bias_prep", {
                    TOFCommandCode::RESET_DAQ,
                    TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_D,
                    TOFCommandCode::ACQUIRE_TDC_CALIBRATION,
                    TOFCommandCode::ACQUIRE_QDC_CALIBRATION,
                    TOFCommandCode::PROCESS_TDC_CALIBRATION,
                    TOFCommandCode::PROCESS_QDC_CALIBRATION
                });
                macroLoopRunning_ = false;
            });
            return true;
        });
    };

    //  MACRO_CYCLIC_RUN_LOOP 
    table_[TOFCommandCode::MACRO_CYCLIC_RUN_LOOP] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::MACRO_CYCLIC_RUN_LOOP, [&]() {
            if (macroLoopRunning_) {
                Logger::instance().warn("[CommandDispatch] Loop macro is already active!");
                return false;
            }
    
            uint32_t cycles = (argv.size() > 0) ? static_cast<uint32_t>(argv[0]) : 0;
            if (argv.size() > 1) {
                double customDuration = static_cast<double>(argv[1]);
                sipmDataAcquisitionTime_.store(customDuration);
            }
            Logger::instance().info("[CommandDispatch] Executing Cyclic Run Loop. Target iteration limit set to: {}", 
                                    (cycles == 0 ? "UNLIMITED (Infinity)" : std::to_string(cycles)));
            Logger::instance().info("[CommandDispatch] Loop configured with SiPM duration: {} seconds", sipmDataAcquisitionTime_.load());
 
            if (macroLoopThread_.joinable()) {
                macroLoopThread_.join();
            }
    
            macroLoopThread_ = std::thread([this, cycles]() {
                this->executeMacroLoop("CyclicRunLoop", "cyclic_run_loop", {
                    TOFCommandCode::RESET_DAQ,
                    TOFCommandCode::READ_TEMPERATURE_SENSORS,
                    TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_D,
                    TOFCommandCode::ACQUIRE_SIPM_DATA,
                    TOFCommandCode::CONVERT_RAW_TO_RAW,
                    TOFCommandCode::CONVERT_STG1_TO_STG2,
                    TOFCommandCode::PROCESS_QA_COIN,
                    TOFCommandCode::PROCESS_QA_IRIDIUM
                }, cycles);
                macroLoopRunning_ = false;
            });
    
            return true;
        });
    };

    // HEART_BEAT
    table_[TOFCommandCode::HEART_BEAT] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            Logger::instance().detail("[CommandDispatch] Received HEART_BEAT from Hub");
            
            // No execution needed, just acknowledge
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            return true;
        } catch (...) {
            Logger::instance().error("[CommandDispatch] Exception in HEART_BEAT");
            return false; // signal failure, though this should almost never happen
        }
    };

    // DUMMY_TEST
    table_[TOFCommandCode::DUMMY_TEST] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            Logger::instance().info("[CommandDispatch] Handling dummy command for testing");
    
            // Optional: simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
            return true; 
        } catch (...) {
            Logger::instance().error("[CommandDispatch] Exception in DUMMY_TEST");
            return false; // signal failure
        }
    };

    monitorRunning_ = true;
    monitorThread_ = std::thread(&GRAMS_TOF_CommandDispatch::runMonitorThread, this);
}

GRAMS_TOF_CommandDispatch::~GRAMS_TOF_CommandDispatch() {
    macroLoopRunning_ = false;
    if (macroLoopThread_.joinable()) {
        macroLoopThread_.join();
    }

    monitorRunning_ = false; 
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }

    daqRunning_ = false;
    if (daqThread_.joinable()) {
        daqThread_.join();
    }

    Logger::instance().error("[CommandDispatch] Exception in destructor");
}

void GRAMS_TOF_CommandDispatch::runDAQThread() {
    try {
        Logger::instance().info("[CommandDispatch] DAQ thread started");
        pyint_.getDAQ().run(); 
        Logger::instance().info("[CommandDispatch] DAQ thread finished");
    } catch (const std::exception& e) {
        Logger::instance().error("[CommandDispatch] Exception in DAQ thread: {}", e.what());
    } catch (...) {
        Logger::instance().error("[CommandDispatch] Unknown exception in DAQ thread");
    }
    
    {
        std::lock_guard<std::mutex> lock(daqMutex_);
        daqRunning_ = false;
    }
}

bool GRAMS_TOF_CommandDispatch::dispatch(TOFCommandCode code, const CommandArgs& argv) {
    auto it = table_.find(code);
    if (it != table_.end()) {
        try {
            return it->second(argv);
        } catch (const std::exception& e) {
            Logger::instance().error("[CommandDispatch] Exception for command {}: {}", static_cast<int>(code), e.what());
            return false;
        } catch (...) {
            Logger::instance().error("[CommandDispatch] Unknown exception for command: {}", static_cast<int>(code));
            return false;
        }
    }
    Logger::instance().error("[CommandDispatch] Unknown command code: {}", static_cast<int>(code));
    return false;
}

bool GRAMS_TOF_CommandDispatch::executeManagedBackground(
    TOFCommandCode code,
    const std::string& scriptName, 
    const std::vector<std::string>& args,
    std::function<void(bool)> postCompletionCallback,
    const std::string& interpreter) 
{
    std::string scriptPath = pyint_.resolveScriptPath(scriptName);

    pid_t pid = fork();
    if (pid == 0) {
        // --- CHILD PROCESS ---
        for (int i = 3; i < 1024; ++i) ::close(i); 

        std::vector<char*> c_args;
        c_args.push_back(const_cast<char*>(interpreter.c_str()));
        c_args.push_back(const_cast<char*>(scriptPath.c_str()));

        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr); 

        execvp(c_args[0], c_args.data());
        _exit(1); 
    } 
    else if (pid > 0) {
        // --- PARENT PROCESS ---
        {
            std::lock_guard<std::mutex> lock(pidMutex_);
            // Register process AND its callback under lock immediately
            activeBackgroundTasks_[pid] = BackgroundTask{code, postCompletionCallback};
        }

        Logger::instance().info("[CommandDispatch] Started {} task (PID: {})", interpreter, pid);
        return true;
    }
    return false;
}

void GRAMS_TOF_CommandDispatch::sendStatusCallback(TOFCommandCode code, uint32_t status) {
    GRAMS_TOF_CommandCodec::Packet cb;
    cb.code = static_cast<uint16_t>(pgrams::communication::CommunicationCodes::TOF_Callback);
    
    // Add the command code and the status to the argument vector
    cb.argv.push_back(static_cast<uint32_t>(code)); 
    cb.argv.push_back(status); // 0 = Success, 1 = Fail, 2 = Interrupted
    cb.argc = cb.argv.size();
   
    const int max_retries = 10;
    const int retry_delay_ms = 100;

    for (int i = 0; i < max_retries; ++i) {
        if (eventClient_.sendPacket(cb)) {
            if (i >= 0) {
                Logger::instance().debug("[CommandDispatch] CALLBACK for 0x{:04X} sent successfully after {} retries.", 
                                          static_cast<uint16_t>(code), i+1);
            }
            return; // Success
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
    }

    Logger::instance().error("[CommandDispatch] Failed to send status callback for 0x{:04X} after {} retries.", 
                             static_cast<uint16_t>(code), max_retries);
}

bool GRAMS_TOF_CommandDispatch::executeSimpleCommand(TOFCommandCode code, std::function<bool()> func) {
    bool success = false;
    try {
        success = func();
    } catch (const std::exception& e) {
        Logger::instance().error("[CommandDispatch] Exception in command 0x{:04X}: {}", static_cast<int>(code), e.what());
        success = false;
    }
    sendStatusCallback(code, success ? 0 : 1);
    return success;
}

bool GRAMS_TOF_CommandDispatch::executeMacroSequence(
    const std::string& macroName, 
    const std::string& vaultSubDir, 
    const std::vector<TOFCommandCode>& sequence) 
{
    Logger::instance().info("[CommandDispatch][{}] Starting macro sequence...", macroName);
    auto& config = GRAMS_TOF_Config::instance();

    macroLoopRunning_ = true;

    if (!vaultSubDir.empty()) {
        std::string timestamp = config.getCurrentTimestamp();
        std::string vaultDir = config.getTOFDataDir() + "/vault/" + vaultSubDir + "/run_" + timestamp; 
        Logger::instance().info("[CommandDispatch][{}] Initializing Vault: {}", macroName, vaultDir);
        try {
            std::filesystem::create_directories(vaultDir);
            config.setVaultPath(vaultDir);
        } catch (const std::exception& e) {
            Logger::instance().error("[CommandDispatch][{}] Could not create vault: {}", macroName, e.what());
            macroLoopRunning_ = false;
            return false;
        }
    }

    for (auto cmd : sequence) {
        if (!macroLoopRunning_) {
            Logger::instance().warn("[CommandDispatch][{}] Stop flag detected mid-sequence. Aborting.", macroName);
            if (!vaultSubDir.empty()) config.clearVaultPath();
            return false;
        }

        Logger::instance().info("[CommandDispatch][{}] Executing step: 0x{:04X}", macroName, static_cast<uint16_t>(cmd));
        
        if (!this->dispatch(cmd, {})) {
            Logger::instance().error("[CommandDispatch][{}] Step 0x{:04X} failed. Aborting macro.", macroName, static_cast<uint16_t>(cmd));
            if (!vaultSubDir.empty()) config.clearVaultPath();
            macroLoopRunning_ = false;
            return false;
        }

        if (cmd == TOFCommandCode::RESET_DAQ) {
          macroLoopRunning_ = true;
        }

        bool is_running = true;
        while (is_running && macroLoopRunning_) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::lock_guard<std::mutex> lock(pidMutex_);
            is_running = false;
            for (auto const& [pid, task] : activeBackgroundTasks_) {
                if (task.commandCode == cmd) {
                    is_running = true;
                    break;
                }
            }
        }
    }

    if (!vaultSubDir.empty()) {
        config.clearVaultPath();
    }

    //macroLoopRunning_ = false;

    Logger::instance().info("[CommandDispatch][{}] Macro sequence completed successfully.", macroName);
    return true;
}

bool GRAMS_TOF_CommandDispatch::executeMacroLoop(
    const std::string& macroName, 
    const std::string& vaultSubDir, 
    const std::vector<TOFCommandCode>& sequence,
    uint32_t numCycles) 
{
    Logger::instance().info("[CommandDispatch][{}] Continuous Loop Initiated. Target cycles: {}", 
                            macroName, (numCycles == 0 ? "Infinity" : std::to_string(numCycles)));
    
    macroLoopRunning_ = true;
    uint32_t currentCycle = 0;

    while (macroLoopRunning_) {
        currentCycle++;
        Logger::instance().info("[CommandDispatch][{}] --- Starting Cycle #{} ---", macroName, currentCycle);

        std::string cycleVaultPath = vaultSubDir.empty() ? "" : vaultSubDir + "/cycle_" + std::to_string(currentCycle);

        if (!this->executeMacroSequence(macroName + "_Cycle", cycleVaultPath, sequence)) {
            Logger::instance().error("[CommandDispatch][{}] Cycle #{} failed. Breaking loop engine.", macroName, currentCycle);
            macroLoopRunning_ = false;
            break;
        }

        if (numCycles > 0 && currentCycle >= numCycles) {
            Logger::instance().info("[CommandDispatch][{}] Target cycle count ({}) reached.", macroName, numCycles);
            macroLoopRunning_ = false;
            break;
        }

        if (macroLoopRunning_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    Logger::instance().warn("[CommandDispatch][{}] Continuous Loop Stopped.", macroName);
    return true;
}

void GRAMS_TOF_CommandDispatch::runMonitorThread() {
    while (monitorRunning_) {
        {
            std::lock_guard<std::mutex> lock(pidMutex_);
            for (auto it = activeBackgroundTasks_.begin(); it != activeBackgroundTasks_.end(); ) {
                int status;
                pid_t pid = it->first;
                const BackgroundTask& task = it->second;

                pid_t result = waitpid(pid, &status, WNOHANG);

                if (result > 0) {
                    bool success = WIFEXITED(status) && (WEXITSTATUS(status) == 0);
                    Logger::instance().info("[CommandDispatch] Process {} for command 0x{:04X} finished with status {}", 
                                             pid, static_cast<int>(task.commandCode), WEXITSTATUS(status));
                    if (task.postCompletionCallback) {
                        task.postCompletionCallback(success);
                    }
                    sendStatusCallback(task.commandCode, success ? 0 : 1);
                    
                    it = activeBackgroundTasks_.erase(it);
                } else if (result == -1) {
                    it = activeBackgroundTasks_.erase(it);
                } else {
                    it++; 
                }
            }
        }

        for (int i = 0; i < 10 && monitorRunning_; ++i) 
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

