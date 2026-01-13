#pragma once
#include <string>

bool runProcessTofCoinEvtQA(const std::string& inputFile, 
                            const std::string& outputBase, 
                            int qdcMode = 1,
                            const std::string& tdcCalibPath = "",
                            const std::string& qdcCalibPath = "");
