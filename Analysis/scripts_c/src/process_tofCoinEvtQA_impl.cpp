#include "process_tofCoinEvtQA.h"

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TPaveStats.h>
#include <TText.h>
#include <TROOT.h>

#include <TOF_CoincidenceEvents.h>
#include <TOF_ChannelConversion.h>
#include <TOF_Fitting.h>
#include <TOF_Attributes.h>
#include <TOF_TdcQdcCalibration.h>

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

bool runProcessTofCoinEvtQA(const std::string& inputFile, 
                            const std::string& outputBase, 
                            int qdcMode,
                            const std::string& tdcCalibPath,
                            const std::string& qdcCalibPath)
{
  //warm up
  gROOT->SetBatch(kTRUE);
  TF1 *f_warmup = new TF1("warmup", "gaus", 0, 1);
  delete f_warmup;
  TCanvas *c_warmup = new TCanvas("c_warmup", "warmup");
  c_warmup->Print("warmup_init.pdf"); 
  delete c_warmup;
  gSystem->Unlink("warmup_init.pdf"); // Remove the dummy file

  /// Class setup
  TOF_CoincidenceEvents* theCoin = new TOF_CoincidenceEvents(); //::getInstance();
  auto theChanConv = TOF_ChannelConversion::getInstance();
  auto theFit      = TOF_Fitting::getInstance();
  auto theAttrib   = TOF_Attributes::getInstance();
  auto theCalib    = TOF_TdcQdcCalibration::getInstance();

  // Logic for Calibration Files
  TString tdcPath = tdcCalibPath;
  TString qdcPath = qdcCalibPath;
  TString glibDir = gSystem->Getenv("GLIB");

  if (tdcPath.IsWhitespace()) {
    tdcPath = Form("%s/config/tdc_calibration.tsv", glibDir.Data());
  }
  if (qdcPath.IsWhitespace()) {
    qdcPath = Form("%s/config/qdc_calibration.tsv", glibDir.Data());
  }

  // Load specified or default calibration files
  printf("[INFO] Loading TDC calibration: %s\n", tdcPath.Data());
  printf("[INFO] Loading QDC calibration: %s\n", qdcPath.Data());
  theCalib->readTdcCalib(tdcPath);
  theCalib->readQdcCalib(qdcPath);

  TString pdfName  = Form("%s.pdf",  outputBase.c_str());
  TString rootName = Form("%s.root", outputBase.c_str());

  TFile* fin = new TFile( inputFile.c_str(), "read" );
  if( !fin->IsOpen() ) {
    printf("[ERR] FILE NOT FOUND: %s", inputFile.c_str());
    return false;
  }

  TOF_TreeData* tD = (TOF_TreeData*) fin->Get("data");
  if( !tD ) return false;
  //tD->setBranchAddress();

  /// ROI channel list
  uint8_t febD_connID = 4; // connector ID on FEB/D. range [1,8].
  std::vector<uint8_t>  smaChannels = { 1, 2, 65, 66 }; // SMA connector IDs
  std::vector<uint32_t> activeChannels;
  for( auto chan: smaChannels )
  {
    auto achanID = theChanConv->getAbsoluteChannelID( febD_connID, chan );
    activeChannels.push_back( achanID );
  }

  std::sort( activeChannels.begin(), activeChannels.end() );
  for( auto chan: activeChannels ) cout << "channel: " << chan << endl;

  TFile *fout = new TFile(rootName, "recreate" );

  /// class setup
  auto ok = theCoin->setTreeData( tD ); cout << ok << endl;
  theCoin->setActiveChannels( activeChannels );
  //theCoin->setQdcCalibMethod( TOF_QdcCalibMethod::fGetEnergy );
  auto tC = theCoin->getCoincidenceEventsTree();

  tC->Print();
  tC->Write();

  //std::vector<TOF_CoincidenceChannelInfo>* vBranch = nullptr;
  //tC->SetBranchAddress( "coinEvt", &vBranch );

  theCoin->generateHistoForQA();

  theCoin->fHisto_dT     ->Write();
  theCoin->fHisto_NbOfEvt->Write();
  theCoin->fHisto_TvsQcal->Write();

  cout << "=================================" << endl;
  cout << "ouput: " << fout->GetName() << endl;
  cout << "=================================" << endl;

  fout->Close();

  return true;
}

