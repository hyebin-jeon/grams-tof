#include "GRAMS_TOF_RootConverter.h"
#include "GRAMS_TOF_MonitorCodec.h"
#include "GRAMS_TOF_CommandCodec.h"
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TRandom.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// STAGE 0: GENERATE
void generateDummyRoot(const std::string& filename) 
{
    std::cout << "[Step 0] Generating Original ROOT file: " << filename << std::endl;
    TFile* f = new TFile(filename.c_str(), "RECREATE");
    
    TH1F* h1 = new TH1F("h_energy", "Energy Deposit", 100, 0, 100);
    for(int i=0; i<1000; ++i) h1->Fill(gRandom->Gaus(50, 10));
    
    TH2F* h2 = new TH2F("h_hitmap", "Hit Map", 10, 0, 10, 10, 0, 10);
    h2->Fill(5, 5, 100);
    
    TProfile* tp = new TProfile("p_timing", "Timing Profile", 50, 0, 50);
    tp->Fill(10, 1.2); tp->Fill(10, 1.4); // Mean 1.3
    
    f->Write();
    f->Close();
}

// STAGE 1: CONVERT (ROOT -> Binary)
void convertRootToBinaries(const std::string& rootFile, uint32_t runNum) 
{
    std::cout << "[Step 1] Converting " << rootFile << " to binary packets..." << std::endl;
    
    auto dataList = GRAMS_TOF_RootConverter::scanFile(rootFile, runNum);
    
    for (const auto& data : dataList) {
        // Wrap in TPC Packet Logic
        auto packet = GRAMS_TOF_MonitorCodec::encode(data);
        auto bytes = GRAMS_TOF_CommandCodec::serialize(packet);

        // Fixed: Use hname for the filename and logging instead of hist_id
        std::string outName = "packet_" + std::string(data.hname) + ".bin";        
        std::ofstream ofs(outName, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        ofs.close();
        
        std::cout << "  -> Created: " << outName << " (Name: " << data.hname << ")" << std::endl;
    }
}


// STAGE 2: RECONSTRUCT (Binary -> ROOT)
void reconstructFromBinaries(const std::string& outputRoot) 
{
    std::cout << "[Step 2] Reconstructing ROOT file: " << outputRoot << std::endl;
    TFile* fout = new TFile(outputRoot.c_str(), "RECREATE");

    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.path().extension() == ".bin" && entry.path().filename().string().find("packet_") == 0) {
            
            std::ifstream ifs(entry.path(), std::ios::binary | std::ios::ate);
            std::streamsize size = ifs.tellg();
            ifs.seekg(0, std::ios::beg);
            std::vector<uint8_t> buffer(size);
            ifs.read(reinterpret_cast<char*>(buffer.data()), size);

            GRAMS_TOF_CommandCodec::Packet tpcPacket;
            if (GRAMS_TOF_CommandCodec::parse(buffer, tpcPacket)) {
                GRAMS_TOF_MonitorCodec::MonitorData monData;
                if (GRAMS_TOF_MonitorCodec::decode(tpcPacket, monData)) {
                    // Fixed: Use the name captured in monData instead of hist_id
                    std::string name = std::string(monData.hname);
                    
                    // Passing monData.hname to toRootObject automatically restores original name
                    TH1* h = GRAMS_TOF_RootConverter::toRootObject(monData, name);
                    h->Write();
                    delete h;
                    std::cout << "  -> Reconstructed: " << monData.hname << std::endl;
                }
            }
        }
    }
    fout->Close();
}

int main(int argc, char** argv) 
{
    if (argc < 2) {
        std::cout << "Usage: ./final_test <mode>\nModes: 0 (Gen), 1 (Convert), 2 (Reconstruct)\n";
        return 1;
    }

    int mode = std::atoi(argv[1]);
    if      (mode == 0) generateDummyRoot("run_2026-02-05_00-31-07.198Z.stg2.coin.root");
    else if (mode == 1) convertRootToBinaries("run_2026-02-05_00-31-07.198Z.stg2.coin.root", 20260210);
    else if (mode == 2) reconstructFromBinaries("run_2026-02-05_00-31-07.198Z.stg2.coin_recon.root");
    
    return 0;
}

