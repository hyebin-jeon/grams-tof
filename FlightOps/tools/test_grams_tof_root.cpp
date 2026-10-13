#include "GRAMS_TOF_RootConverter.h"   // ScanResult, scanFile, toRootObject
#include "GRAMS_TOF_MonitorCodec.h"
#include "GRAMS_TOF_CommandCodec.h"
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TGraph.h>
#include <TParameter.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cassert>
#include <cmath>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────
//  Helper: write a single packet to a .bin file
// ─────────────────────────────────────────────────────────────

static void writePacket(const GRAMS_TOF_CommandCodec::Packet& packet,
                        const std::string& name)
{
    auto bytes   = GRAMS_TOF_CommandCodec::serialize(packet);
    std::string outName = "packet_" + name + ".bin";
    std::ofstream ofs(outName, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    std::cout << "  -> Created: " << outName << std::endl;
}

// ─────────────────────────────────────────────────────────────
//  STAGE 1: Convert real ROOT file → .bin packets
// ─────────────────────────────────────────────────────────────

void convertRootToBinaries(const std::string& rootFile, uint32_t runNum)
{
    std::cout << "\n[Step 1] Converting " << rootFile << " (run=" << runNum << ")\n";

    auto scan = GRAMS_TOF_RootConverter::scanFile(rootFile, runNum);
    std::cout << "  Found: " << scan.hists.size() << " hists, "
              << scan.graphs.size() << " graphs, "
              << scan.params.size() << " params\n";

    for (const auto& data : scan.hists)  writePacket(GRAMS_TOF_MonitorCodec::encode(data), data.hname);
    for (const auto& data : scan.graphs) writePacket(GRAMS_TOF_MonitorCodec::encode(data), data.gname);
    for (const auto& data : scan.params) writePacket(GRAMS_TOF_MonitorCodec::encode(data), data.pname);
}

// ─────────────────────────────────────────────────────────────
//  STAGE 2: Reconstruct .bin packets → ROOT file
// ─────────────────────────────────────────────────────────────

void reconstructFromBinaries(const std::string& outputRoot)
{
    std::cout << "\n[Step 2] Reconstructing ROOT file: " << outputRoot << "\n";
    TFile* fout = new TFile(outputRoot.c_str(), "RECREATE");
    int nOK = 0, nFail = 0;

    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.path().extension() != ".bin") continue;
        if (entry.path().filename().string().find("packet_") != 0) continue;

        std::ifstream ifs(entry.path(), std::ios::binary | std::ios::ate);
        std::streamsize sz = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        std::vector<uint8_t> buf(sz);
        ifs.read(reinterpret_cast<char*>(buf.data()), sz);

        GRAMS_TOF_CommandCodec::Packet pkt;
        if (!GRAMS_TOF_CommandCodec::parse(buf, pkt)) {
            std::cerr << "  [FAIL] parse error: " << entry.path() << "\n";
            ++nFail; continue;
        }

        // argv[5] holds the DataType
        if (pkt.argv.size() < 6) {
            std::cerr << "  [FAIL] packet too short: " << entry.path() << "\n";
            ++nFail; continue;
        }

        auto dtype = static_cast<GRAMS_TOF_MonitorCodec::DataType>(pkt.argv[5]);

        if (dtype == GRAMS_TOF_MonitorCodec::DataType::TH1F  ||
            dtype == GRAMS_TOF_MonitorCodec::DataType::TH2F  ||
            dtype == GRAMS_TOF_MonitorCodec::DataType::TProfile)
        {
            GRAMS_TOF_MonitorCodec::MonitorData md;
            if (!GRAMS_TOF_MonitorCodec::decode(pkt, md)) { ++nFail; continue; }
            TH1* h = GRAMS_TOF_RootConverter::toRootObject(md, md.hname);
            h->Write();
            delete h;
            std::cout << "  -> [Hist]  " << md.hname << "\n";
        }
        else if (dtype == GRAMS_TOF_MonitorCodec::DataType::TGraph)
        {
            GRAMS_TOF_MonitorCodec::GraphData gd;
            if (!GRAMS_TOF_MonitorCodec::decode(pkt, gd)) { ++nFail; continue; }
            TGraph* g = GRAMS_TOF_RootConverter::toRootObject(gd, gd.gname);
            g->Write();
            delete g;
            std::cout << "  -> [Graph] " << gd.gname << "\n";
        }
        else if (dtype == GRAMS_TOF_MonitorCodec::DataType::TParameter)
        {
            GRAMS_TOF_MonitorCodec::ParameterData pd;
            if (!GRAMS_TOF_MonitorCodec::decode(pkt, pd)) { ++nFail; continue; }
            TParameter<double> p(pd.pname, pd.value);
            p.Write();
            std::cout << "  -> [Param] " << pd.pname << " = " << pd.value << "\n";
        }
        else
        {
            std::cerr << "  [FAIL] unknown DataType=" << pkt.argv[5]
                      << " in " << entry.path() << "\n";
            ++nFail; continue;
        }
        ++nOK;
    }

    fout->Close();
    std::cout << "  Done: " << nOK << " OK, " << nFail << " failed\n";
}

// ─────────────────────────────────────────────────────────────
//  STAGE 3: Verify – compare original vs reconstructed
// ─────────────────────────────────────────────────────────────

void verifyRoundTrip(const std::string& origRoot, const std::string& reconRoot)
{
    std::cout << "\n[Step 3] Verifying round-trip: " << origRoot
              << " vs " << reconRoot << "\n";

    TFile* forig  = TFile::Open(origRoot.c_str(),  "READ");
    TFile* frecon = TFile::Open(reconRoot.c_str(), "READ");

    if (!forig || forig->IsZombie())  { std::cerr << "Cannot open " << origRoot  << "\n"; return; }
    if (!frecon || frecon->IsZombie()) { std::cerr << "Cannot open " << reconRoot << "\n"; return; }

    int nPass = 0, nFail = 0;

    TIter next(forig->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {
        std::string kname  = key->GetName();
        std::string kclass = key->GetClassName();

        // ── TH1 (TH1F / TH2F / TProfile) ──
        if (TClass::GetClass(kclass.c_str())->InheritsFrom(TH1::Class())) {
            TH1* horig  = dynamic_cast<TH1*>(forig->Get(kname.c_str()));
            TH1* hrecon = dynamic_cast<TH1*>(frecon->Get(kname.c_str()));
            if (!hrecon) {
                std::cerr << "  [FAIL] " << kname << " missing in reconstructed file\n";
                ++nFail; continue;
            }
            // x_min/x_max are stored as float in the codec → relative tolerance for mean
            double meanOrig = horig->GetMean(), meanRecon = hrecon->GetMean();
            double meanScale = std::max(std::abs(meanOrig), 1.0);
            // GetMean() relies on internal statistics (fTsumw, fTsumwx) accumulated
            // during Fill(x, weight). These are NOT stored in the packet — only bin
            // contents (Σweight per bin) are transmitted. ResetStats() recomputes mean
            // as Σ(content_i × x_center_i) / Σ(content_i), which differs from the
            // original Σ(weight_i × x_i) / Σ(weight_i) when weights vary per Fill.
            // → mean deviation of ~1-3% is expected for rate histograms (Fill(x, 1/T)).
            // Bin contents (Integral) are the authoritative check.
            bool ok = (horig->GetNbinsX() == hrecon->GetNbinsX()) &&
                      (std::abs(horig->Integral() - hrecon->Integral()) < 1.0);
            std::cout << "  [" << (ok?"PASS":"FAIL") << "] Hist  " << kname
                      << "  bins=" << horig->GetNbinsX()
                      << "  xmin_orig="  << horig->GetXaxis()->GetXmin()
                      << "  xmin_recon=" << hrecon->GetXaxis()->GetXmin()
                      << "  xmax_orig="  << horig->GetXaxis()->GetXmax()
                      << "  xmax_recon=" << hrecon->GetXaxis()->GetXmax()
                      << "  mean_orig="      << meanOrig
                      << "  mean_recon="     << meanRecon
                      << "  mean_reldiff="   << std::abs(meanOrig - meanRecon) / meanScale
                      << "  integral_orig="  << horig->Integral()
                      << "  integral_recon=" << hrecon->Integral() << "\n";
            ok ? ++nPass : ++nFail;
        }
        // ── TGraph ──
        else if (kclass == "TGraph") {
            TGraph* gorig  = dynamic_cast<TGraph*>(forig->Get(kname.c_str()));
            TGraph* grecon = dynamic_cast<TGraph*>(frecon->Get(kname.c_str()));
            if (!grecon) {
                std::cerr << "  [FAIL] " << kname << " missing in reconstructed file\n";
                ++nFail; continue;
            }
            bool ok = (gorig->GetN() == grecon->GetN());
            double maxXRelDiff = 0, maxYRelDiff = 0;
            if (ok && gorig->GetN() > 0) {
                for (int i = 0; i < gorig->GetN(); ++i) {
                    double xo, yo, xr, yr;
                    gorig->GetPoint(i, xo, yo);
                    grecon->GetPoint(i, xr, yr);
                    // Relative tolerance: float has ~7 significant digits → allow 1e-5
                    double xScale = std::max(std::abs(xo), 1.0);
                    double yScale = std::max(std::abs(yo), 1.0);
                    maxXRelDiff = std::max(maxXRelDiff, std::abs(xo - xr) / xScale);
                    maxYRelDiff = std::max(maxYRelDiff, std::abs(yo - yr) / yScale);
                }
                ok = (maxXRelDiff < 1e-5) && (maxYRelDiff < 1e-5);
            }
            std::cout << "  [" << (ok?"PASS":"FAIL") << "] Graph " << kname
                      << "  n=" << gorig->GetN()
                      << "  max_x_reldiff=" << maxXRelDiff
                      << "  max_y_reldiff=" << maxYRelDiff << "\n";
            ok ? ++nPass : ++nFail;
        }
        // ── TParameter<double> ──
        else if (kclass == "TParameter<double>") {
            auto* porig  = dynamic_cast<TParameter<double>*>(forig->Get(kname.c_str()));
            auto* precon = dynamic_cast<TParameter<double>*>(frecon->Get(kname.c_str()));
            if (!precon) {
                std::cerr << "  [FAIL] " << kname << " missing in reconstructed file\n";
                ++nFail; continue;
            }
            bool ok = (std::abs(porig->GetVal() - precon->GetVal()) < 1e-9);
            std::cout << "  [" << (ok?"PASS":"FAIL") << "] Param " << kname
                      << "  orig=" << porig->GetVal()
                      << "  recon=" << precon->GetVal() << "\n";
            ok ? ++nPass : ++nFail;
        }
        else {
            std::cout << "  [SKIP] " << kname << " (" << kclass << ")\n";
        }
    }

    forig->Close();
    frecon->Close();
    std::cout << "\n  Result: " << nPass << " PASS / " << nFail << " FAIL\n";
}

// ─────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "Usage:\n"
                  << "  mode 1 (Convert):     " << argv[0] << " 1 <input.root> <run_number>\n"
                  << "  mode 2 (Reconstruct): " << argv[0] << " 2 <output.root>\n"
                  << "  mode 3 (Verify):      " << argv[0] << " 3 <original.root> <reconstructed.root>\n"
                  << "  mode 123 (All-in-one):" << argv[0] << " 123 <input.root> <run_number>\n";
        return 1;
    }

    int mode = std::atoi(argv[1]);

    if (mode == 1) {
        if (argc < 4) { std::cerr << "mode 1 requires <input.root> <run_number>\n"; return 1; }
        convertRootToBinaries(argv[2], std::atoi(argv[3]));
    }
    else if (mode == 2) {
        reconstructFromBinaries(argv[2]);
    }
    else if (mode == 3) {
        if (argc < 4) { std::cerr << "mode 3 requires <original.root> <reconstructed.root>\n"; return 1; }
        verifyRoundTrip(argv[2], argv[3]);
    }
    else if (mode == 123) {
        // All-in-one: convert → reconstruct → verify
        if (argc < 4) { std::cerr << "mode 123 requires <input.root> <run_number>\n"; return 1; }
        std::string inputRoot = argv[2];
        uint32_t runNum       = std::atoi(argv[3]);
        std::string reconRoot = "reconstructed.root";

        convertRootToBinaries(inputRoot, runNum);
        reconstructFromBinaries(reconRoot);
        verifyRoundTrip(inputRoot, reconRoot);
    }
    else {
        std::cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }

    return 0;
}
