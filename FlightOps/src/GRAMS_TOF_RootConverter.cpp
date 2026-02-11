#include "GRAMS_TOF_RootConverter.h"
#include <cstring>

std::vector<GRAMS_TOF_MonitorCodec::MonitorData> GRAMS_TOF_RootConverter::scanFile(const std::string& path, uint32_t runNum) {
    std::vector<GRAMS_TOF_MonitorCodec::MonitorData> results;
    TFile* f = TFile::Open(path.c_str(), "READ");
    if (!f || f->IsZombie()) return results;

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {
        TObject* obj = key->ReadObj();
        if (obj->InheritsFrom(TH1::Class())) {
            results.push_back(fromRootObject(static_cast<TH1*>(obj), runNum));
        }
        delete obj; // Avoid memory leaks during scan
    }
    f->Close();
    delete f;
    return results;
}

GRAMS_TOF_MonitorCodec::MonitorData GRAMS_TOF_RootConverter::fromRootObject(TH1* h, uint32_t runNum) {
    GRAMS_TOF_MonitorCodec::MonitorData data;
    data.run_number = runNum;
    
    std::memset(data.hname, 0, 16);
    std::strncpy(data.hname, h->GetName(), 15);

    data.n_bins_x = h->GetNbinsX();
    data.x_min = (float)h->GetXaxis()->GetXmin();
    data.x_max = (float)h->GetXaxis()->GetXmax();

    if (h->InheritsFrom(TProfile::Class())) {
        data.hist_type = 3; 
        data.n_bins_y = 1;
    } else if (h->InheritsFrom(TH2::Class())) {
        data.hist_type = 2;
        TH2* h2 = static_cast<TH2*>(h);
        data.n_bins_y = h2->GetNbinsY();
        data.y_min = (float)h2->GetYaxis()->GetXmin();
        data.y_max = (float)h2->GetYaxis()->GetXmax();
    } else {
        data.hist_type = 1;
        data.n_bins_y = 1;
    }

    packBins(h, data.bins, data.hist_type);
    return data;
}

TH1* GRAMS_TOF_RootConverter::toRootObject(const GRAMS_TOF_MonitorCodec::MonitorData& data, const std::string& fallbackName) {
    // 1. Determine Name: Use the name from the packet if available, otherwise use fallback
    std::string hName = (std::strlen(data.hname) > 0) ? data.hname : fallbackName;
    TH1* h = nullptr;

    // 2. Handle TProfile (hist_type == 3)
    if (data.hist_type == 3) {
        TProfile* p = new TProfile(hName.c_str(), hName.c_str(), 
                                   data.n_bins_x, data.x_min, data.x_max);
        p->Sumw2();

        for (int i = 1; i <= (int)data.n_bins_x; ++i) {
            float f_m, f_r, f_e;
            // Extract Mean, RMS, and Entries triplets
            std::memcpy(&f_m, &data.bins[(i-1) * 3],     sizeof(float));
            std::memcpy(&f_r, &data.bins[(i-1) * 3 + 1], sizeof(float));
            std::memcpy(&f_e, &data.bins[(i-1) * 3 + 2], sizeof(float));
        
            if (f_e > 0) {
                p->SetBinContent(i, f_m * f_e); 
                p->SetBinEntries(i, f_e);
                // Reconstruct Sumw2 to preserve error bars correctly
                double sumW2 = (double)f_r * f_r * f_e + (double)f_m * f_m * f_e;
                p->GetSumw2()->SetAt(sumW2, i);
            }
        }
        h = p;
    } 
    // 3. Handle TH2F (hist_type == 2)
    else if (data.hist_type == 2) {
        TH2F* h2 = new TH2F(hName.c_str(), hName.c_str(), 
                            data.n_bins_x, data.x_min, data.x_max, 
                            data.n_bins_y, data.y_min, data.y_max);

        // Include underflow/overflow bins (nx+2 * ny+2)
        int nx = data.n_bins_x + 2;
        int ny = data.n_bins_y + 2;
        for (int i = 0; i < nx * ny; ++i) {
            float val;
            std::memcpy(&val, &data.bins[i], sizeof(float));
            h2->SetBinContent(i, val);
        }
        h = h2;
    } 
    // 4. Handle TH1F (Default / hist_type == 1)
    else {
        TH1F* h1 = new TH1F(hName.c_str(), hName.c_str(), 
                            data.n_bins_x, data.x_min, data.x_max);

        for (int i = 0; i < (int)data.bins.size(); ++i) {
            float val;
            std::memcpy(&val, &data.bins[i], sizeof(float));
            h1->SetBinContent(i, val);
        }
        h = h1;
    }

    return h;
}

uint32_t GRAMS_TOF_RootConverter::generateHistID(const char* name) {
    // Simple Jenkins hash or similar to keep ID unique but deterministic
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
    if (type == 3) { // TProfile Logic
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
    } else { // TH1F and TH2F Logic
        int totalBins = (h->GetNbinsX() + 2) * (h->GetNbinsY() + 2);
        for (int i = 0; i < totalBins; ++i) {
            float val = (float)h->GetBinContent(i);
            uint32_t raw;
            std::memcpy(&raw, &val, sizeof(float));
            bins.push_back(raw);
        }
    }
}
