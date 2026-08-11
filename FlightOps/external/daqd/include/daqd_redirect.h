#pragma once

#include <cstdio>
#include <cstdarg>

extern "C" int daqd_custom_printf(const char* format, ...);
extern "C" int daqd_custom_fprintf(std::FILE* stream, const char* format, ...);

#define printf daqd_custom_printf
#define fprintf daqd_custom_fprintf
