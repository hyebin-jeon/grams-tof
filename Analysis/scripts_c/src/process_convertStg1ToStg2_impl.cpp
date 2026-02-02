#include "process_convertStg1ToStg2.h"

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

#include <TOF_ConvertStg1toStg2.h>

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

bool runConvertStg1ToStg2(const std::string& inputFile, 
                          const std::string& outputBase )
{
  ////warm up
  //gROOT->SetBatch(kTRUE);
  //TF1 *f_warmup = new TF1("warmup", "gaus", 0, 1);
  //delete f_warmup;
  //TCanvas *c_warmup = new TCanvas("c_warmup", "warmup");
  //c_warmup->Print("warmup_init.pdf"); 
  //delete c_warmup;
  //gSystem->Unlink("warmup_init.pdf"); // Remove the dummy file
  
	/// Class setup
	auto theConvStg = TOF_ConvertStg1toStg2::getInstance();

	/// output naming
  const char* inputFile_c = inputFile.c_str();	
	TString name_root = std::filesystem::path(inputFile_c).filename().c_str();
	if( !name_root.EndsWith(".stg1.root") ) {
		std::cerr<< "[ERR] Wrong Input File. Provide *.stg1.root" << std::endl;
		return false;
	}

	TString name_file = (TString) name_root(0, name_root.Index( ".stg1.root" ));
	TString name_dir  = std::filesystem::path(inputFile_c).parent_path().c_str();
	TString outputDir = outputBase.empty()==true? name_dir : (TString) outputBase;
	  
	theConvStg->convertStg1ToStg2( inputFile_c, Form("%s/%s.stg2.root", outputDir.Data(), name_file.Data()) );

  return true;
}

