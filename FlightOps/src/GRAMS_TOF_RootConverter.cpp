#include "GRAMS_TOF_RootConverter.h"
#include "GRAMS_TOF_Logger.h"
#include "GRAMS_TOF_RuntimeError.h"
#include <cstring>

// ─────────────────────────────────────────────────────────────
//  Internal helper
// ─────────────────────────────────────────────────────────────

TFile* GRAMS_TOF_RootConverter::openFile(const std::string& path) {
    TFile* f = TFile::Open(path.c_str(), "READ");
    if (!f || f->IsZombie()) {
        Logger::instance().error(
            "[RootConverter] Failed to open target ROOT file asset or file is corrupted: {}", path);
        return nullptr;
    }
    return f;
}

// ─────────────────────────────────────────────────────────────
//  scanFile  (single pass over the file, all object types)
// ─────────────────────────────────────────────────────────────

GRAMS_TOF_RootConverter::ScanResult
GRAMS_TOF_RootConverter::scanFile(const std::string& path, uint32_t runNum) {
    ScanResult result;
    TFile* f = openFile(path);
    if (!f) return result;

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {
        TObject* obj = key->ReadObj();
        if (!obj) continue;

        if (obj->InheritsFrom(TH1::Class())) {
            result.hists.push_back(fromRootObject(static_cast<TH1*>(obj), runNum));
        } else if (obj->InheritsFrom(TGraph::Class())) {
            result.graphs.push_back(fromRootObject(static_cast<TGraph*>(obj), runNum));
        } else if (std::string(key->GetClassName()) == "TParameter<double>") {
            result.params.push_back(fromRootObject(static_cast<TParameter<double>*>(obj), runNum));
        } else {
            Logger::instance().debug(
                "[RootConverter] scanFile: skipping '{}' ({})",
                key->GetName(), obj->ClassName());
        }
        delete obj;
    }
    f->Close();
    delete f;
    return result;
}

// ─────────────────────────────────────────────────────────────
//  fromRootObject overloads
// ─────────────────────────────────────────────────────────────

GRAMS_TOF_MonitorCodec::MonitorData
GRAMS_TOF_RootConverter::fromRootObject(TH1* h, uint32_t runNum) {
    GRAMS_TOF_MonitorCodec::MonitorData data;
    data.run_number = runNum;

    std::memset(data.hname, 0, 16);
    std::strncpy(data.hname, h->GetName(), 15);

    data.n_bins_x = h->GetNbinsX();
    data.x_min = (float)h->GetXaxis()->GetXmin();
    data.x_max = (float)h->GetXaxis()->GetXmax();

    if (h->InheritsFrom(TProfile::Class())) {
        data.hist_type = static_cast<uint32_t>(GRAMS_TOF_MonitorCodec::DataType::TProfile);
        data.n_bins_y = 1;
    } else if (h->InheritsFrom(TH2::Class())) {
        data.hist_type = static_cast<uint32_t>(GRAMS_TOF_MonitorCodec::DataType::TH2F);
        TH2* h2 = static_cast<TH2*>(h);
        data.n_bins_y = h2->GetNbinsY();
        data.y_min = (float)h2->GetYaxis()->GetXmin();
        data.y_max = (float)h2->GetYaxis()->GetXmax();
    } else {
        data.hist_type = static_cast<uint32_t>(GRAMS_TOF_MonitorCodec::DataType::TH1F);
        data.n_bins_y = 1;
    }

    packBins(h, data.bins, data.hist_type);
    return data;
}

GRAMS_TOF_MonitorCodec::GraphData
GRAMS_TOF_RootConverter::fromRootObject(TGraph* g, uint32_t runNum) {
    GRAMS_TOF_MonitorCodec::GraphData data;
    data.run_number = runNum;

    std::memset(data.gname, 0, 16);
    std::strncpy(data.gname, g->GetName(), 15);

    data.n_points = (uint32_t)g->GetN();
    data.x.resize(data.n_points);
    data.y.resize(data.n_points);

    for (int i = 0; i < (int)data.n_points; ++i) {
        g->GetPoint(i, data.x[i], data.y[i]);
    }
    return data;
}

GRAMS_TOF_MonitorCodec::ParameterData
GRAMS_TOF_RootConverter::fromRootObject(TParameter<double>* p, uint32_t runNum) {
    GRAMS_TOF_MonitorCodec::ParameterData data;
    data.run_number = runNum;

    std::memset(data.pname, 0, 16);
    std::strncpy(data.pname, p->GetName(), 15);

    data.value = p->GetVal();
    return data;
}

// ─────────────────────────────────────────────────────────────
//  toRootObject overloads
// ─────────────────────────────────────────────────────────────

TH1* GRAMS_TOF_RootConverter::toRootObject(const GRAMS_TOF_MonitorCodec::MonitorData& data,
                                            const std::string& fallbackName) {
    std::string hName = (std::strlen(data.hname) > 0) ? data.hname : fallbackName;
    TH1* h = nullptr;

    const auto type = static_cast<GRAMS_TOF_MonitorCodec::DataType>(data.hist_type);

    if (type == GRAMS_TOF_MonitorCodec::DataType::TProfile) {
        TProfile* p = new TProfile(hName.c_str(), hName.c_str(),
                                   data.n_bins_x, data.x_min, data.x_max);
        p->Sumw2();
        for (int i = 1; i <= (int)data.n_bins_x; ++i) {
            float f_m, f_r, f_e;
            std::memcpy(&f_m, &data.bins[(i-1)*3],     sizeof(float));
            std::memcpy(&f_r, &data.bins[(i-1)*3 + 1], sizeof(float));
            std::memcpy(&f_e, &data.bins[(i-1)*3 + 2], sizeof(float));
            if (f_e > 0) {
                p->SetBinContent(i, f_m * f_e);
                p->SetBinEntries(i, f_e);
                double sumW2 = (double)f_r * f_r * f_e + (double)f_m * f_m * f_e;
                p->GetSumw2()->SetAt(sumW2, i);
            }
        }
        h = p;
    } else if (type == GRAMS_TOF_MonitorCodec::DataType::TH2F) {
        TH2F* h2 = new TH2F(hName.c_str(), hName.c_str(),
                            data.n_bins_x, data.x_min, data.x_max,
                            data.n_bins_y, data.y_min, data.y_max);
        int nx = data.n_bins_x + 2;
        int ny = data.n_bins_y + 2;
        for (int i = 0; i < nx * ny; ++i) {
            float val;
            std::memcpy(&val, &data.bins[i], sizeof(float));
            h2->SetBinContent(i, val);
        }
        h2->ResetStats();
        h = h2;
    } else if (type == GRAMS_TOF_MonitorCodec::DataType::TH1F) {
        TH1F* h1 = new TH1F(hName.c_str(), hName.c_str(),
                            data.n_bins_x, data.x_min, data.x_max);
        for (int i = 0; i < (int)data.bins.size(); ++i) {
            float val;
            std::memcpy(&val, &data.bins[i], sizeof(float));
            h1->SetBinContent(i, val);
        }
        // SetBinContent does not update internal statistics (fTsumw, fTsumwx, ...).
        // ResetStats forces GetMean()/GetRMS() to recompute from bin contents.
        h1->ResetStats();
        h = h1;
    } else {
        throw GRAMS_TOF_RuntimeError(
            fmt::format("[RootConverter] Unsupported hist_type in MonitorData: {}", data.hist_type));
    }

    return h;
}

TGraph* GRAMS_TOF_RootConverter::toRootObject(const GRAMS_TOF_MonitorCodec::GraphData& data,
                                               const std::string& fallbackName) {
    std::string gName = (std::strlen(data.gname) > 0) ? data.gname : fallbackName;
    TGraph* g = new TGraph((int)data.n_points);
    g->SetName(gName.c_str());
    g->SetTitle(gName.c_str());

    for (int i = 0; i < (int)data.n_points; ++i) {
        g->SetPoint(i, data.x[i], data.y[i]);
    }
    return g;
}

// ─────────────────────────────────────────────────────────────
//  Private helpers
// ─────────────────────────────────────────────────────────────

uint32_t GRAMS_TOF_RootConverter::generateHistID(const char* name) {
    uint32_t hash = 0;
    while (*name) {
        hash += *name++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

void GRAMS_TOF_RootConverter::packBins(TH1* h, std::vector<uint32_t>& bins, uint32_t type) {
    if (type == static_cast<uint32_t>(GRAMS_TOF_MonitorCodec::DataType::TProfile)) {
        TProfile* p = static_cast<TProfile*>(h);
        for (int i = 1; i <= p->GetNbinsX(); ++i) {
            float mean = (float)p->GetBinContent(i);
            float rms  = (float)p->GetBinError(i);
            float ent  = (float)p->GetBinEntries(i);
            uint32_t u_m, u_r, u_e;
            std::memcpy(&u_m, &mean, sizeof(float));
            std::memcpy(&u_r, &rms,  sizeof(float));
            std::memcpy(&u_e, &ent,  sizeof(float));
            bins.push_back(u_m);
            bins.push_back(u_r);
            bins.push_back(u_e);
        }
    } else { // TH1F and TH2F
        int totalBins = (h->GetNbinsX() + 2) * (h->GetNbinsY() + 2);
        for (int i = 0; i < totalBins; ++i) {
            float val = (float)h->GetBinContent(i);
            uint32_t raw;
            std::memcpy(&raw, &val, sizeof(float));
            bins.push_back(raw);
        }
    }
}
