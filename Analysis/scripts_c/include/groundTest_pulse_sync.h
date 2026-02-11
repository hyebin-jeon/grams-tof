#pragma once
#include <string>

bool runGroundTestPulseSync(const std::string& inputFile, 
                           const std::string& outputBase = "", 
													 const int febD_connID = -1,
													 const int pulseChA = -1,
													 const int pulseChB = -1 );
