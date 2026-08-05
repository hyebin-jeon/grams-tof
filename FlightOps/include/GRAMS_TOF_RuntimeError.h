#pragma once

#include <stdexcept>
#include <string>
#include "GRAMS_TOF_Logger.h"

class GRAMS_TOF_RuntimeError : public std::runtime_error {
public:
    explicit GRAMS_TOF_RuntimeError(const std::string& message) : std::runtime_error(message) {
        Logger::instance().error("{}", message);
    }
};
