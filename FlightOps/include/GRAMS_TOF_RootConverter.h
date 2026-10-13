#pragma once

#include "GRAMS_TOF_MonitorCodec.h"
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TGraph.h>
#include <TParameter.h>
#include <TKey.h>
#include <string>
#include <vector>

class GRAMS_TOF_RootConverter {
public:
    // ── Unified scan result ───────────────────────────────────────────────
    struct ScanResult {
        std::vector<GRAMS_TOF_MonitorCodec::MonitorData>   hists;
        std::vector<GRAMS_TOF_MonitorCodec::GraphData>     graphs;
        std::vector<GRAMS_TOF_MonitorCodec::ParameterData> params;

        bool empty() const { return hists.empty() && graphs.empty() && params.empty(); }
        std::size_t size() const { return hists.size() + graphs.size() + params.size(); }
    };

    // ── Single scan entry point, all object types ─────────────────────────
    static ScanResult scanFile(const std::string& path, uint32_t runNum);

    // ── From ROOT object → codec struct ───────────────────────────────────
    static GRAMS_TOF_MonitorCodec::MonitorData
        fromRootObject(TH1* h, uint32_t runNum);

    static GRAMS_TOF_MonitorCodec::GraphData
        fromRootObject(TGraph* g, uint32_t runNum);

    static GRAMS_TOF_MonitorCodec::ParameterData
        fromRootObject(TParameter<double>* p, uint32_t runNum);

    // ── From codec struct → ROOT object ───────────────────────────────────
    static TH1*    toRootObject(const GRAMS_TOF_MonitorCodec::MonitorData&   data,
                                const std::string& fallbackName);

    static TGraph* toRootObject(const GRAMS_TOF_MonitorCodec::GraphData&     data,
                                const std::string& fallbackName);

private:
    static TFile* openFile(const std::string& path);
    static uint32_t generateHistID(const char* name);
    static void packBins(TH1* h, std::vector<uint32_t>& bins, uint32_t type);
};
