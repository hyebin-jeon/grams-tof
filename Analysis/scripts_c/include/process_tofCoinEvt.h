#pragma once
#include <string>

bool runProcessTofCoinEvt(const std::string& inputFile, 
                          const std::string& outputBase, 
                          int qdcMode = 1,
                          const std::string& tdcCalibPath = "",
                          const std::string& qdcCalibPath = "");
