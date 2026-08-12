#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <chrono>
#include <iomanip>

#include <fmt/format.h>

namespace LogColor {
    constexpr auto RESET   = "\033[0m";
    constexpr auto RED     = "\033[31m";
    constexpr auto GREEN   = "\033[32m";
    constexpr auto YELLOW  = "\033[33m";
    constexpr auto BLUE    = "\033[34m";
    constexpr auto GRAY    = "\033[90m";
    constexpr auto WHITE   = "\033[97m"; 
    constexpr auto CYAN    = "\033[36m";   
    constexpr auto MAGENTA = "\033[35m";
}

class Logger {
public:
    enum class Level { Detail = 0, Debug = 1, Info = 2, Warning = 3, Error = 4, Critical = 5 };

    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void setLogFile(const std::string& logfile) {
        std::lock_guard<std::mutex> lock(mutex_);
        logFile_ = std::make_unique<std::ofstream>(logfile);
    }

    void setLogLevel(Level level) {
        std::lock_guard<std::mutex> lock(mutex_);
        minLevel_ = level;
    }

    // Base log call handling an explicit component string
    template<typename... Args>
    void log_comp(Level level, const std::string& component, const std::string& fmt_str, Args&&... args) {
        if (static_cast<int>(level) < static_cast<int>(minLevel_)) return;

        std::lock_guard<std::mutex> lock(mutex_);
        const char* color = getColor(level);
        const char* label = getLabel(level);
        std::string timestamp = getTimestamp();
        std::string message_content = fmt::format(fmt_str, std::forward<Args>(args)...);

        // Strip any legacy wrapping brackets from the component tag if passed down explicitly
        std::string comp_clean = component;
        if (!comp_clean.empty() && comp_clean.front() == '[') comp_clean.erase(0, 1);
        if (!comp_clean.empty() && comp_clean.back() == ']')  comp_clean.pop_back();

        std::string file_line = fmt::format(
            "{} [{:<4}] {:<15} ❯ {}\n",
            timestamp, label, comp_clean, message_content
        );

        // Build a high-contrast colored string string layout optimized for terminals
        std::string console_line = fmt::format(
            "{} [{}{:<4}{}] {:<15} ❯ {}\n",
            timestamp, color, label, LogColor::RESET, comp_clean, message_content
        );

        // Output to terminal
        std::cout << console_line;

        // Output to file
        if (logFile_ && logFile_->is_open()) {
            (*logFile_) << file_line;
            logFile_->flush();
        }
    }

    // Fallback log parser that parses out legacy "[Component] message" patterns automatically
    template<typename... Args>
    void log(Level level, const std::string& fmt_str, Args&&... args) {
        std::string evaluated_msg = fmt::format(fmt_str, std::forward<Args>(args)...);
        
        // Check if the incoming string starts with a traditional "[Component] " layout
        if (!evaluated_msg.empty() && evaluated_msg.front() == '[') {
            size_t close_bracket = evaluated_msg.find(']');
            if (close_bracket != std::string::npos && close_bracket > 1) {
                std::string comp = evaluated_msg.substr(1, close_bracket - 1);
                std::string actual_msg = evaluated_msg.substr(close_bracket + 1);
                
                // Trim leading whitespace off remaining message
                size_t first_char = actual_msg.find_first_not_of(" \t");
                if (first_char != std::string::npos) {
                    actual_msg = actual_msg.substr(first_char);
                }
                
                log_comp(level, comp, "{}", actual_msg);
                return;
            }
        }

        // Default component fallback if no component signature is detected
        log_comp(level, "System", "{}", evaluated_msg);
    }

    // Modern Shorthand API Supporting Explicit Component Declarations
    template<typename... Args>
    void info(const std::string& comp, const std::string& fmt, Args&&... args) {
        log_comp(Level::Info, comp, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void info(const std::string& fmt, Args&&... args) {
        log(Level::Info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(const std::string& comp, const std::string& fmt, Args&&... args) {
        log_comp(Level::Warning, comp, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void warn(const std::string& fmt, Args&&... args) {
        log(Level::Warning, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const std::string& comp, const std::string& fmt, Args&&... args) {
        log_comp(Level::Error, comp, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void error(const std::string& fmt, Args&&... args) {
        log(Level::Error, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(const std::string& comp, const std::string& fmt, Args&&... args) {
        log_comp(Level::Debug, comp, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void debug(const std::string& fmt, Args&&... args) {
        log(Level::Debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void detail(const std::string& comp, const std::string& fmt, Args&&... args) {
        log_comp(Level::Detail, comp, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void detail(const std::string& fmt, Args&&... args) {
        log(Level::Detail, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(const std::string& comp, const std::string& fmt, Args&&... args) {
        log_comp(Level::Critical, comp, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void critical(const std::string& fmt, Args&&... args) {
        log(Level::Critical, fmt, std::forward<Args>(args)...);
    }

private:
    Logger() = default;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    const char* getColor(Level level) const {
        using namespace LogColor;
        switch (level) {
            case Level::Critical: return MAGENTA;
            case Level::Info:     return WHITE;
            case Level::Warning:  return YELLOW;
            case Level::Error:    return RED;
            case Level::Debug:    return GREEN;
            case Level::Detail:   return CYAN;
        }
        return RESET;
    }

    const char* getLabel(Level level) const {
        switch (level) {
            case Level::Critical: return "CRIT";
            case Level::Info:     return "INFO";
            case Level::Warning:  return "WARN";
            case Level::Error:    return "ERR "; 
            case Level::Debug:    return "DBG ";
            case Level::Detail:   return "DTL ";
        }
        return "LOG";
    }

    std::unique_ptr<std::ofstream> logFile_;
    Level minLevel_ = Level::Info;
    std::mutex mutex_;
};
