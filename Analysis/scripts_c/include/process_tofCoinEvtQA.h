#pragma once
#include <string>

bool runProcessTofCoinEvtQA(const std::string& inputFile, 
                            const std::string& outputBase   = "", 
                            const std::string& tdcCalibPath = "",
                            const std::string& qdcCalibPath = "");
