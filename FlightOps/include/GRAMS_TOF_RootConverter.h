#pragma once

#include "GRAMS_TOF_MonitorCodec.h"
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TKey.h>
#include <string>
#include <vector>

class GRAMS_TOF_RootConverter {
public:
    static std::vector<GRAMS_TOF_MonitorCodec::MonitorData> scanFile(const std::string& path, uint32_t runNum);
    static GRAMS_TOF_MonitorCodec::MonitorData fromRootObject(TH1* h, uint32_t runNum);
    static TH1* toRootObject(const GRAMS_TOF_MonitorCodec::MonitorData& data, const std::string& name);

private:
    static uint32_t generateHistID(const char* name);
    static void packBins(TH1* h, std::vector<uint32_t>& bins, uint32_t type);
};
