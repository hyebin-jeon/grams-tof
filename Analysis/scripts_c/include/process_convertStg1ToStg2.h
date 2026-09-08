#pragma once
#include <string>

bool runConvertStg1ToStg2(const std::string& inputFile, 
                          const std::string& outputBase="",
                          const std::string& tdc_cal   ="",
                          const std::string& qdc_cal   ="" );
                          //const std::string& asic_list ="");
