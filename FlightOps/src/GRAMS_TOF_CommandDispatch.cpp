#include "GRAMS_TOF_CommandDispatch.h"
#include "GRAMS_TOF_Logger.h"
#include "GRAMS_TOF_Config.h"
#include <unistd.h>
#include <sys/wait.h>

GRAMS_TOF_CommandDispatch::GRAMS_TOF_CommandDispatch(
    GRAMS_TOF_PythonIntegration& pyint,
    GRAMS_TOF_Analyzer& analyzer,
    GRAMS_TOF_EventClient& eventClient)
    : pyint_(pyint), analyzer_(analyzer), eventClient_(eventClient), table_{}
{
    auto& config = GRAMS_TOF_Config::instance();

    // START_DAQ
    table_[TOFCommandCode::START_DAQ] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::START_DAQ, [&]() {
            Logger::instance().warn("[Dispatch] Starting DAQ...");
            std::lock_guard<std::mutex> lock(daqMutex_);
    
            if (daqRunning_) return false;
    
            if (daqThread_.joinable()) {
                daqThread_.join();
            }
    
            if (!pyint_.getDAQ().initialize()) {
                Logger::instance().error("Failed to re-initialize DAQ Manager");
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
            Logger::instance().warn("[STOP] Initiating Master Stop Sequence...");
    
            // 1. HARDWARE STOP (Trigger this IMMEDIATELY)
            std::string stopScriptPath = pyint_.resolveScriptPath("stop_daq.py");
            pid_t stop_pid = fork();
            if (stop_pid == 0) {
                for (int i = 3; i < 1024; ++i) ::close(i);
                execlp("python3", "python3", stopScriptPath.c_str(), nullptr);
                _exit(1);
            } else if (stop_pid > 0) {
                signal(SIGCHLD, SIG_IGN);
                Logger::instance().info("[STOP] Hardware stop script triggered (PID: {}).", stop_pid);
            }
    
            // 2. INTERRUPT MANAGED BACKGROUND PROCESSES
            {
                std::lock_guard<std::mutex> lock(pidMutex_);
                for (auto const& [b_pid, b_code] : activeBackgroundPIDs_) {
                    Logger::instance().warn("[STOP] Terminating {} (PID: {})", static_cast<int>(b_code), b_pid);
                    kill(b_pid, SIGTERM);
                    sendStatusCallback(b_code, 2); 
                }
                activeBackgroundPIDs_.clear();
            }
    
            // 3. SIGNAL SOFTWARE STOP
            pyint_.getDAQ().stop();
    
            // 4. JOIN THREAD 
            if (daqThread_.joinable()) {
                daqThread_.join();
                Logger::instance().warn("[STOP] DAQ thread joined.");
            }
    
            {
                std::lock_guard<std::mutex> lock(daqMutex_);
                daqRunning_ = false;
            }
    
            Logger::instance().info("[STOP] Full system interrupt complete.");
            return true;
        });
    };

    // RESET_DAQ
    table_[TOFCommandCode::RESET_DAQ] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RESET_DAQ, [&]() {
            Logger::instance().warn("[RESET] Initiating Master Reset (Matching Stop Sequence)...");
        
            // 1. HARDWARE STOP (Triggered IMMEDIATELY - Matches STOP_DAQ Step 1)
            std::string stopScriptPath = pyint_.resolveScriptPath("stop_daq.py");
            pid_t stop_pid = fork();
            if (stop_pid == 0) {
                for (int i = 3; i < 1024; ++i) ::close(i);
                execlp("python3", "python3", stopScriptPath.c_str(), nullptr);
                _exit(1);
            } else if (stop_pid > 0) {
                signal(SIGCHLD, SIG_IGN);
                Logger::instance().info("[RESET] Hardware stop script triggered (PID: {}).", stop_pid);
            }
        
            // 2. INTERRUPT MANAGED BACKGROUND PROCESSES (Matches STOP_DAQ Step 2)
            {
                std::lock_guard<std::mutex> lock(pidMutex_);
                for (auto const& [b_pid, b_code] : activeBackgroundPIDs_) {
                    Logger::instance().warn("[RESET] Terminating {} (PID: {})", static_cast<int>(b_code), b_pid);
                    kill(b_pid, SIGTERM);
                    sendStatusCallback(b_code, 2); 
                }
                activeBackgroundPIDs_.clear();
            }
        
            // 3. SIGNAL SOFTWARE STOP (Matches STOP_DAQ Step 3)
            pyint_.getDAQ().stop();
        
            // 4. JOIN THREAD (Matches STOP_DAQ Step 4)
            if (daqThread_.joinable()) {
                daqThread_.join();
                Logger::instance().warn("[RESET] DAQ thread joined.");
            }
        
            // --- RESET SPECIFIC ADDITIONS (Re-initialization) ---
            // 5. MANAGER CLEANUP
            pyint_.getDAQ().cleanup(); 
                
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
        
            // 6. RE-INITIALIZE
            if (!pyint_.getDAQ().initialize()) {
                Logger::instance().error("[RESET] Failed to re-initialize DAQ Manager");
                return false;
            }
        
            {
                std::lock_guard<std::mutex> lock(daqMutex_);
                daqRunning_ = false;
            }
        
            Logger::instance().info("[RESET] System reset complete. Ready for new START.");
            return true;
        });
    };

    // RUN_INIT_SYSTEM
    table_[TOFCommandCode::RUN_INIT_SYSTEM] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_INIT_SYSTEM, [&]() {
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Executing init_system.py script...");
            return pyint_.runPetsysInitSystem("scripts.init_system");
        });
    };

    // RUN_MAKE_BIAS_CALIB_TABLE
    table_[TOFCommandCode::RUN_MAKE_BIAS_CALIB_TABLE] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_MAKE_BIAS_CALIB_TABLE, [&]() {
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Executing make_bias_calibration_table.py script...");
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

    // RUN_MAKE_SIMPLE_BIAS_SET_TABLE
    table_[TOFCommandCode::RUN_MAKE_SIMPLE_BIAS_SET_TABLE] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_MAKE_SIMPLE_BIAS_SET_TABLE, [&]() {
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Executing make_simple_bias_settings_table.py script...");
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

    // RUN_MAKE_SIMPLE_CHANNEL_MAP
    table_[TOFCommandCode::RUN_MAKE_SIMPLE_CHANNEL_MAP] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_MAKE_SIMPLE_CHANNEL_MAP, [&]() {
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Executing make_simple_channel_map.py script...");
            return pyint_.runPetsysMakeSimpleChannelMap(
                "scripts.make_simple_channel_map",
                config.getConfigDir() + "/map"
            );
        });
    };

    // RUN_MAKE_SIMPLE_DISC_SET_TABLE
    table_[TOFCommandCode::RUN_MAKE_SIMPLE_DISC_SET_TABLE] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_MAKE_SIMPLE_DISC_SET_TABLE, [&]() {
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Executing make_simple_disc_settings_table.py script...");
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

    // RUN_READ_TEMPERATURE_SENSORS
    table_[TOFCommandCode::RUN_READ_TEMPERATURE_SENSORS] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Executing read_temperature_sensors.py script...");
            std::vector<std::string> sArgs;
            sArgs.push_back("--time");
            sArgs.push_back(std::to_string(argv.size() > 0 ? static_cast<double>(argv[0]) : 0.0));
            sArgs.push_back("--interval");
            sArgs.push_back(std::to_string(argv.size() > 1 ? static_cast<double>(argv[1]) : 60.0));
            sArgs.push_back("-o");
            sArgs.push_back("/dev/null");
            bool startup = (argv.size() > 2) ? (argv[2] != 0) : true;
            if (startup) sArgs.push_back("--startup");
            bool debug = (argv.size() > 3) ? (argv[3] != 0) : false;
            if (debug) sArgs.push_back("--debug");

            return executeManagedBackground(TOFCommandCode::RUN_READ_TEMPERATURE_SENSORS, "read_temperature_sensors.py", sArgs);
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in RUN_READ_TEMPERATURE_SENSORS");
            return false;
        }
    };

    // RUN_ACQUIRE_THRESHOLD_CALIBRATION
    table_[TOFCommandCode::RUN_ACQUIRE_THRESHOLD_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().warn("[Dispatch] Starting full threshold calibration in background...");
            
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

            return executeManagedBackground(TOFCommandCode::RUN_ACQUIRE_THRESHOLD_CALIBRATION, "acquire_threshold_calibration.py", sArgs);
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in RUN_ACQUIRE_THRESHOLD_CALIBRATION");
            return false;
        }
    };

    // RUN_ACQUIRE_THRESHOLD_CALIBRATION_BN (Baseline and Noise only) ---
    table_[TOFCommandCode::RUN_ACQUIRE_THRESHOLD_CALIBRATION_BN] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().warn("[Dispatch] Starting Baseline/Noise calibration in background...");
            
            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getCalibrationDir(), "disc_calibration", timestampStr));
            sArgs.push_back("--mode");
            sArgs.push_back("baseline_noise");
            sArgs.push_back("--nreads-noise");
            sArgs.push_back(std::to_string(argv.size() > 0 ? argv[0] : 4));
            bool bias = (argv.size() > 2) ? (argv[2] != 0) : false;
            if (bias) sArgs.push_back("--ext-bias");

            return executeManagedBackground(TOFCommandCode::RUN_ACQUIRE_THRESHOLD_CALIBRATION_BN, "acquire_threshold_calibration.py", sArgs);
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in RUN_ACQUIRE_THRESHOLD_CALIBRATION_BN");
            return false;
        }
    };
    
    // RUN_ACQUIRE_THRESHOLD_CALIBRATION_D (Dark counts only) ---
    table_[TOFCommandCode::RUN_ACQUIRE_THRESHOLD_CALIBRATION_D] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().warn("[Dispatch] Starting Baseline/Noise calibration in background...");
            
            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getCalibrationDir(), "disc_calibration", timestampStr));
            sArgs.push_back("--mode");
            sArgs.push_back("dark");
            sArgs.push_back("--nreads-noise");
            sArgs.push_back(std::to_string(argv.size() > 0 ? argv[0] : 4));
            bool bias = (argv.size() > 2) ? (argv[2] != 0) : false;
            if (bias) sArgs.push_back("--ext-bias");

            return executeManagedBackground(TOFCommandCode::RUN_ACQUIRE_THRESHOLD_CALIBRATION_BN, "acquire_threshold_calibration.py", sArgs);
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in RUN_ACQUIRE_THRESHOLD_CALIBRATION_D");
            return false;
        }
    };

		// RUN_ACQUIRE_ACTIVE_ASIC_LIST - hyeb
    table_[TOFCommandCode::RUN_ACQUIRE_ACTIVE_ASIC_LIST] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            //auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().warn("[Dispatch] Starting acquiring active asic list in background...");

            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.config.getConfigDir(), "active_asic")); //, timestampStr));

            return executeManagedBackground(TOFCommandCode::RUN_ACQUIRE_ACTIVE_ASIC_LIST, "acquire_active_asic_list.py", sArgs);
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in RUN_ACQUIRE_ACTIVE_ASIC_LIST");
            return false;
        }
    };

    // RUN_ACQUIRE_QDC_CALIBRATION
    table_[TOFCommandCode::RUN_ACQUIRE_QDC_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().warn("[Dispatch] Starting QDC calibration in background...");

            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getCalibrationDir(), "qdc_calibration", timestampStr));

            return executeManagedBackground(TOFCommandCode::RUN_ACQUIRE_QDC_CALIBRATION, "acquire_qdc_calibration.py", sArgs);
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in RUN_ACQUIRE_QDC_CALIBRATION");
            return false;
        }
    };

    // RUN_ACQUIRE_TDC_CALIBRATION
    table_[TOFCommandCode::RUN_ACQUIRE_TDC_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().warn("[Dispatch] Starting TDC calibration in background...");

            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getCalibrationDir(), "tdc_calibration", timestampStr));

            return executeManagedBackground(TOFCommandCode::RUN_ACQUIRE_TDC_CALIBRATION, "acquire_tdc_calibration.py", sArgs);
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in RUN_ACQUIRE_TDC_CALIBRATION");
            return false;
        }
    };

    // RUN_ACQUIRE_SIPM_DATA
    table_[TOFCommandCode::RUN_ACQUIRE_SIPM_DATA] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            auto timestampStr = config.getCurrentTimestamp();
            Logger::instance().warn("[Dispatch] Starting SiPM data acquisition in background...");

            std::vector<std::string> sArgs;
            sArgs.push_back("--config");
            sArgs.push_back(config.getConfigFilePath());
            sArgs.push_back("-o");
            sArgs.push_back(config.makeFilePathWithTimestamp(config.getSTG0Dir(), "run", timestampStr));
            sArgs.push_back("--time");
            sArgs.push_back(std::to_string(argv.size() > 0 ? static_cast<double>(argv[0]) : 60.0));
            sArgs.push_back("--mode");
            sArgs.push_back("qdc"); // Defaulting to QDC mode as per your previous logic
            bool hw_trig = (argv.size() > 1) ? (argv[1] != 0) : false;
            if (hw_trig) sArgs.push_back("--enable-hw-trigger");
            
            return executeManagedBackground(TOFCommandCode::RUN_ACQUIRE_SIPM_DATA, "acquire_sipm_data.py", sArgs);
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in RUN_ACQUIRE_SIPM_DATA");
            return false;
        }
    };

    // RUN_PROCESS_THRESHOLD_CALIBRATION
    table_[TOFCommandCode::RUN_PROCESS_THRESHOLD_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_PROCESS_THRESHOLD_CALIBRATION, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getCalibrationDir(), "disc_calibration", "_noise.tsv");
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Running threshold calibration...");
            auto output = analyzer_.runPetsysProcessThresholdCalibration(
                config.getConfigFilePath(),
                config.makeFilePathWithTimestamp(config.getCalibrationDir(), "disc_calibration", timestampStr),
                config.makeFilePathWithTimestamp(config.getDiscDir(), "disc_calibration", timestampStr, "tsv"),
                config.makeFilePathWithTimestamp(config.getCalibrationDir(), "disc_calibration", timestampStr, "root")
            );
            config.copyOrLink(config.getFileByTimestamp(config.getDiscDir(), "disc_calibration", timestampStr),
                              config.getAbsolutePath("main", "disc_calibration_table"), true);
            return output;
        });
    };

    // RUN_PROCESS_TDC_CALIBRATION
    table_[TOFCommandCode::RUN_PROCESS_TDC_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_PROCESS_TDC_CALIBRATION, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getCalibrationDir(), "tdc_calibration");
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Running TDC calibration...");
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
            config.copyOrLink(config.getFileByTimestamp(config.getTDCDir(), "tdc_calibration", timestampStr),
                              config.getAbsolutePath("main", "tdc_calibration_table"), true);
            return output;
        });
    };

    // RUN_PROCESS_QDC_CALIBRATION
    table_[TOFCommandCode::RUN_PROCESS_QDC_CALIBRATION] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_PROCESS_QDC_CALIBRATION, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getCalibrationDir(), "qdc_calibration");
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Running QDC calibration...");
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
            config.copyOrLink(config.getFileByTimestamp(config.getQDCDir(), "qdc_calibration", timestampStr),
                              config.getAbsolutePath("main", "qdc_calibration_table"), true);
            return output;
        });
    };


    // RUN_CONVERT_RAW_TO_RAW
    table_[TOFCommandCode::RUN_CONVERT_RAW_TO_RAW] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_CONVERT_RAW_TO_RAW, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getSTG0Dir(), "run");
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Converting raw to raw...");
            long long eventFractionToWrite = argv.size() > 1 ? static_cast<long long>(argv[0]) : 1024;

            return analyzer_.runPetsysConvertRawToRaw(
                config.getConfigFilePath(),
                config.makeFilePathWithTimestamp(config.getSTG0Dir(), "run", timestampStr),
                config.makeFilePathWithTimestamp(config.getSTG1Dir(), "run", timestampStr, "stg1.root"),
                eventFractionToWrite
            );
        });
    };

    // RUN_CONVERT_RAW_TO_SINGLES
    table_[TOFCommandCode::RUN_CONVERT_RAW_TO_SINGLES] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_CONVERT_RAW_TO_SINGLES, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getSTG0Dir(), "run");
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Converting raw to singles...");
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

    // RUN_CONVERT_STG1_TO_STG2
    table_[TOFCommandCode::RUN_CONVERT_STG1_TO_STG2] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_CONVERT_STG1_TO_STG2, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getSTG1Dir(), "run");
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Converting stg1 to stg2...");
            return analyzer_.runPetsysConvertStg1ToStg2(
								config.getFileByTimestamp(config.getSTG1Dir(), "run", timestampStr),
                config.getSTG2Dir()
            );
        });
    };

    // RUN_PROCESS_TOF_COIN_EVT
    table_[TOFCommandCode::RUN_PROCESS_TOF_COIN_EVT_QA] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        return executeSimpleCommand(TOFCommandCode::RUN_PROCESS_TOF_COIN_EVT_QA, [&]() {
            auto timestampStr = config.getLatestTimestamp(config.getSTG2Dir(), "run");
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Running TOF coin evt calculation...");
            return analyzer_.runPetsysProcessTofCoinEvtQA(
                config.getFileByTimestamp(config.getSTG2Dir(), "run", timestampStr),
								config.getSTG2Dir(),
                config.getString("main", "tdc_calibration_table"),
                config.getString("main", "qdc_calibration_table"),
                {argv.size() > 0 ? static_cast<int>(argv[0]) : -1}
            );
        });
    };

    // HEART_BEAT
    table_[TOFCommandCode::HEART_BEAT] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            Logger::instance().detail("[GRAMS_TOF_CommandDispatch] Received HEART_BEAT from Hub");
            
            // No execution needed, just acknowledge
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            return true;
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in HEART_BEAT");
            return false; // signal failure, though this should almost never happen
        }
    };

    // DUMMY_TEST
    table_[TOFCommandCode::DUMMY_TEST] = [&](const GRAMS_TOF_CommandDispatch::CommandArgs& argv) {
        try {
            Logger::instance().warn("[GRAMS_TOF_CommandDispatch] Handling dummy command for testing");
    
            // Optional: simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
            return true; 
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in DUMMY_TEST");
            return false; // signal failure
        }
    };

    monitorRunning_ = true;
    monitorThread_ = std::thread(&GRAMS_TOF_CommandDispatch::runMonitorThread, this);
}

GRAMS_TOF_CommandDispatch::~GRAMS_TOF_CommandDispatch() {
    try {
        if (daqRunning_) {
            pyint_.getDAQ().stop();
            if (daqThread_.joinable()) {
                daqThread_.join();
            }
        }
    } catch (...) {
        Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in destructor");
    }
}

void GRAMS_TOF_CommandDispatch::runDAQThread() {
    try {
        Logger::instance().warn("[GRAMS_TOF_CommandDispatch] DAQ thread started");
        pyint_.getDAQ().run(); 
        Logger::instance().warn("[GRAMS_TOF_CommandDispatch] DAQ thread finished");
    } catch (const std::exception& e) {
        Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception in DAQ thread: {}", e.what());
    } catch (...) {
        Logger::instance().error("[GRAMS_TOF_CommandDispatch] Unknown exception in DAQ thread");
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
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Exception for command {}: {}", static_cast<int>(code), e.what());
            return false;
        } catch (...) {
            Logger::instance().error("[GRAMS_TOF_CommandDispatch] Unknown exception for command: {}", static_cast<int>(code));
            return false;
        }
    }
    Logger::instance().error("[GRAMS_TOF_CommandDispatch] Unknown command code: {}", static_cast<int>(code));
    return false;
}

bool GRAMS_TOF_CommandDispatch::executeManagedBackground(
    TOFCommandCode code,
    const std::string& scriptName, 
    const std::vector<std::string>& args,
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

        // execvp will look in PATH for "python3" or "root"
        execvp(c_args[0], c_args.data());
        _exit(1); 
    } 
    else if (pid > 0) {
        // --- PARENT PROCESS ---
        {
            std::lock_guard<std::mutex> lock(pidMutex_);
            activeBackgroundPIDs_[pid] = code;
        }
        
        // Ignore child signals to prevent zombies
        //signal(SIGCHLD, SIG_IGN); 
        
        Logger::instance().info("[Dispatch] Started {} task (PID: {})", interpreter, pid);
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
                Logger::instance().debug("[Dispatch] CALLBACK for 0x{:04X} sent successfully after {} retries.", 
                                          static_cast<uint16_t>(code), i+1);
            }
            return; // Success
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
    }

    Logger::instance().error("[Dispatch] Failed to send status callback for 0x{:04X} after {} retries.", 
                             static_cast<uint16_t>(code), max_retries);
}

bool GRAMS_TOF_CommandDispatch::executeSimpleCommand(TOFCommandCode code, std::function<bool()> func) {
    bool success = false;
    try {
        success = func();
    } catch (const std::exception& e) {
        Logger::instance().error("[Dispatch] Exception in command 0x{:04X}: {}", static_cast<int>(code), e.what());
        success = false;
    }
    sendStatusCallback(code, success ? 0 : 1);
    return success;
}

void GRAMS_TOF_CommandDispatch::runMonitorThread() {
    while (monitorRunning_) {
        {
            std::lock_guard<std::mutex> lock(pidMutex_);
            for (auto it = activeBackgroundPIDs_.begin(); it != activeBackgroundPIDs_.end(); ) {
                int status;
                pid_t pid = it->first;
                TOFCommandCode commandCode = it->second;

                pid_t result = waitpid(pid, &status, WNOHANG);

                if (result > 0) {
                    bool success = WIFEXITED(status) && (WEXITSTATUS(status) == 0);
                    Logger::instance().info("[Dispatch] CALLBACK of background process {} for command 0x{:04X} finished with status {}", 
                                             pid, static_cast<int>(commandCode), WEXITSTATUS(status));
                    sendStatusCallback(commandCode, success ? 0 : 1);
                    it = activeBackgroundPIDs_.erase(it);
                } else if (result == -1) {
                    it = activeBackgroundPIDs_.erase(it);
                } else {
                    it++; 
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

