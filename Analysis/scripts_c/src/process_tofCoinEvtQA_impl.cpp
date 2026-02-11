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
                            const std::string& tdcCalibPath,
                            const std::string& qdcCalibPath,
														const int febD_connID_)
{
	/// connector ID
	int febD_connID_default = 1;
	int febD_connID = febD_connID_<0? febD_connID_default : febD_connID_;

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
  printf("[INFO] Load TDC calibration: %s\n", tdcPath.Data());
  printf("[INFO] Load QDC calibration: %s\n", qdcPath.Data());
  theCalib->readTdcCalib(tdcPath);
  theCalib->readQdcCalib(qdcPath);

	/// ROI channel list
	//uint8_t febD_connID = 4; // connector ID on FEB/D. range [1,8].
	std::vector<uint8_t>  smaChannels = { 1, 2, 65, 66 }; // SMA connector IDs
	std::vector<uint32_t> activeChannels;
	for( auto chan: smaChannels )
	{
		auto achanID = theChanConv->getAbsoluteChannelID( febD_connID, chan );
		activeChannels.push_back( achanID );
	}
	std::sort( activeChannels.begin(), activeChannels.end() );
	for( auto chan: activeChannels ) cout << "Coincidence channel: " << chan << endl;

	/// class setup
	const char* inputFile_c = inputFile.c_str();
	if( theCoin->setInputPathStg2( inputFile_c ) != TOF_GOOD ) {
		return 1;
	}
	theCoin->setActiveChannels( activeChannels );

	/// output naming
	TString name_root = std::filesystem::path(inputFile_c).filename().c_str();

	if( !name_root.EndsWith(".stg2.root") ) {
		std::cerr<< "[ERR] Wrong Input File. Provide *.stg2.root" << std::endl;
		return 1;
	}

	TString name_file = (TString) name_root(0, name_root.Index( ".stg2.root" ));
	TString name_dir  = std::filesystem::path(inputFile_c).parent_path().c_str();

	TString outputDir = outputBase.empty()==true? name_dir : (TString) outputBase;

  TString pdfName  = Form("%s/%s.stg2.coin.pdf",  outputDir.Data(), name_file.Data());
  TString rootName = Form("%s/%s.stg2.coin.root", outputDir.Data(), name_file.Data());

	TFile *fout = new TFile(rootName, "recreate" );

	auto tC = theCoin->getCoincidenceEventsTree();
	tC->Write();

  theCoin->generateHistoForQA(pdfName);

  if( theCoin->getHisto_TimeResol()       !=nullptr ) theCoin->getHisto_TimeResol()       ->Write();
  if( theCoin->getHisto_ChannelVsNevents()!=nullptr ) theCoin->getHisto_ChannelVsNevents()->Write();
  if( theCoin->getHisto_TdcVsQcalib()     !=nullptr ) theCoin->getHisto_TdcVsQcalib()     ->Write();
  
	fout->Close();

	std::cout << "[INFO] Generated Output File: " << fout->GetName() << endl;

  return 0;
}

