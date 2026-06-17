#pragma once

// 1. Include the actual, original system standard library header first using absolute paths
#include_next <stdio.h>

// 2. Include your flight logger header
#include "GRAMS_TOF_Logger.h"
#include <string_view>
#include <cstdarg>

// 3. Define the string routing pipeline
inline void daqd_system_log_forwarder(const char* buffer, int length) {
    if (length <= 0 || !buffer) return;
    
    std::string_view view(buffer, length);
    while (!view.empty() && (view.back() == '\n' || view.back() == '\r')) {
        view.remove_suffix(1);
    }
    
    if (!view.empty()) {
        Logger::instance().info("[Petsys Core] {}", view);
    }
}

// 4. Override printf globally using inline matching
inline int daqd_wrapped_printf(const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    daqd_system_log_forwarder(buffer, result);
    return printf("%s", buffer); // Invokes the underlying system symbol safely
}

inline int daqd_wrapped_fprintf(std::FILE* stream, const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (stream == stdout || stream == stderr) {
        daqd_system_log_forwarder(buffer, result);
    }
    return fprintf(stream, "%s", buffer);
}

// 5. Apply the replacements securely
#define printf daqd_wrapped_printf
#define fprintf daqd_wrapped_fprintf
