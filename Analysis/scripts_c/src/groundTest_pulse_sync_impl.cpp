#include "groundTest_pulse_sync.h"

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
#include <TLatex.h>

#include <TOF_TreeDataStg2.h>
#include <TOF_ChannelConversion.h>
#include <TOF_Attributes.h>


#include <filesystem>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

bool runGroundTestPulseSync( const std::string& inputFile, 
                             const std::string& outputBase,
														 const int febD_connID_,
														 const int febS_connID_A_,
														 const int febS_connID_B_
														 )
{
  const char* inputFile_c = inputFile.c_str();	
	TString name_root = std::filesystem::path(inputFile_c).filename().c_str();
	if( !name_root.EndsWith(".stg2.root") ) {
		std::cerr<< "[ERR] Wrong Input File. Provide *.stg2.root" << std::endl;
		return false;
	}
  
	/// Class setup
  auto theChanConv = TOF_ChannelConversion::getInstance();
  auto theAttrib   = TOF_Attributes::getInstance();

	/// pulse channel
	int febD_connID   = febD_connID_  <0? 1 : febD_connID_  ;
	int febS_connID_A = febS_connID_A_<0? 77: febS_connID_A_; // from PPS
	int febS_connID_B = febS_connID_B_<0? 35: febS_connID_B_; // from TPC
  uint32_t chA = theChanConv->getAbsoluteChannelID( febD_connID, febS_connID_A );
  uint32_t chB = theChanConv->getAbsoluteChannelID( febD_connID, febS_connID_B );

	/// output naming
	TString name_file = (TString) name_root(0, name_root.Index( ".stg2.root" ));
	TString name_dir  = std::filesystem::path(inputFile_c).parent_path().c_str();
	TString outputDir = outputBase.empty()==true? name_dir : (TString) outputBase;
	
	TString pdfName = Form("%s/%s.pulse_sync.stg2.pdf", outputDir.Data(), name_file.Data());
	  
	TOF_TreeDataStg2* stg2 = new TOF_TreeDataStg2();
	stg2->setInputPath( inputFile.c_str() );
	if( stg2->getEntries() == 0 ) {
		cerr<< "[ERR] No Entries in Stg2. Terminate." << endl;
		return 0;
	}

	stg2->setBranchStatus("*",1);
	stg2->setBranchAddress();

	vector<TTimeStamp> vTimeA;
	vector<TTimeStamp> vTimeB;

	stg2->getEntry(0);
	TTimeStamp t_begin = stg2->getTimeStampCPU();
	stg2->getEntry( stg2->getEntries()-1 );
	TTimeStamp t_end = stg2->getTimeStampCPU();
	cout << Form("t_begin: (double) %2.2E, (date) %d, (time) %d, (sec) %ld, (nano-sec) %d", t_begin.AsDouble(), t_begin.GetDate(), t_begin.GetTime(), t_begin.GetSec(), t_begin.GetNanoSec()) << endl;
	cout << Form("t_end  : (double) %2.2E, (date) %d, (time) %d, (sec) %ld, (nano-sec) %d", t_end  .AsDouble(), t_end  .GetDate(), t_end  .GetTime(), t_end  .GetSec(), t_end  .GetNanoSec()) << endl;

	for( int i=0; i<stg2->getEntries(); i++ )
	{
		stg2->getEntry(i);

		uint32_t channelID = stg2->getChannelID();
		auto time = stg2->getTimeStampCPU();

		if( channelID == chA ){ vTimeA.push_back( stg2->getTimeStampCPU() );
	    //cout << Form("time_A: (double) %2.5E, (date) %d, (time) %d, (sec) %ld, (nano-sec) %d", time.AsDouble(), time.GetDate(), time.GetTime(), time.GetSec(), time.GetNanoSec() ) << endl;
		}
		if( channelID == chB ){ vTimeB.push_back( stg2->getTimeStampCPU() );
	    //cout << Form("time_B: (double) %2.5E, (date) %d, (time) %d, (sec) %ld, (nano-sec) %d", time.AsDouble(), time.GetDate(), time.GetTime(),time.GetSec(), time.GetNanoSec() ) << endl;
		}
	}

	gStyle->SetOptStat(111111);
	//TTimeStamp t_offset( 1*60, 0); 
	TTimeStamp tmin( t_begin.GetSec(), 0 );
	TTimeStamp tmax( t_end.GetSec()+1, 0 );
	int tBin = (tmax.GetSec() - tmin.GetSec());
	TH1D* hdT   = new TH1D("hdT", "dT (us)", 400, -1, 1 );
	TH1D* hppsA = new TH1D(Form("hpps_ch%03d_J%03d", chA, febS_connID_A), Form("CPU_ch%03d_J%03d", chA, febS_connID_A), tBin, tmin, tmax );
	TH1D* hppsB = new TH1D(Form("hpps_ch%03d_J%03d", chB, febS_connID_B), Form("CPU_ch%03d_J%03d", chB, febS_connID_B), tBin, tmin, tmax );

	hdT->GetXaxis()->SetTitle( Form("t (J%03d) - t_ref (J%03d)", febS_connID_B, febS_connID_A) );
	hppsA->GetXaxis()->SetTitle("CPU time, 1 sec/bin");
	hppsB->GetXaxis()->SetTitle("CPU time, 1 sec/bin");
	hppsA->GetYaxis()->SetTitle("Event rate (Hz)");
	hppsB->GetYaxis()->SetTitle("Event rate (Hz)");

	for( int i=0; i<vTimeA.size(); i++ )
	{
		auto timeA = vTimeA.at(i);
		hppsA->Fill( timeA.AsDouble() );
	}
	for( int i=0; i<vTimeB.size(); i++ )
	{
		auto timeB = vTimeB.at(i);
		hppsB->Fill( timeB.AsDouble() );
	}
	
	hppsA->GetXaxis()->SetTimeDisplay(1);
	hppsA->GetXaxis()->SetTimeFormat("%H:%M:%S");
	hppsA->GetXaxis()->SetTimeOffset(0,"gmt");
	hppsB->GetXaxis()->SetTimeDisplay(1);
	hppsB->GetXaxis()->SetTimeFormat("%H:%M:%S");
	hppsB->GetXaxis()->SetTimeOffset(0,"gmt");

	TCanvas* c1 = new TCanvas("c1", "c1");
	c1->Print( Form("%s[", pdfName.Data()) ); // open
	c1->cd();
	hppsA->Draw();
	hppsA->GetXaxis()->SetNdivisions( 1010 );
	gPad->Modified();
	gPad->Update();
	c1->Print( pdfName.Data() );
	hppsB->Draw();
	hppsB->GetXaxis()->SetNdivisions( 1010 );
	gPad->Modified();
	gPad->Update();
	c1->Print( pdfName.Data() );

	if( vTimeA.size() == vTimeB.size() )
	{
	  for( int i=0; i<vTimeA.size(); i++ )
	  {
	  	auto timeA = vTimeA.at(i);
	  	auto timeB = vTimeB.at(i);
	  	long long dt_sec  = timeB.GetSec()     - timeA.GetSec();
      long long dt_nano = timeB.GetNanoSec() - timeA.GetNanoSec();
      double dT = dt_sec + dt_nano * 1e-9;
	  	hdT->Fill( dT*1E6 ); // in us

			if( i<100 ) {
	    cout << Form("[%03d] time_A: (double) %2.5E, (date) %d, (time) %d, (sec) %ld, (nano-sec) %d", i, timeA.AsDouble(), timeA.GetDate(), timeA.GetTime(), timeA.GetSec(), timeA.GetNanoSec() ) << endl;
	    cout << Form("[%03d] time_B: (double) %2.5E, (date) %d, (time) %d, (sec) %ld, (nano-sec) %d", i, timeB.AsDouble(), timeB.GetDate(), timeB.GetTime(), timeB.GetSec(), timeB.GetNanoSec() ) << endl;
			}
		}

		double mean = hdT->GetMean();
		double sigma = hdT->GetRMS();
		double factor = 5.0;
		double xmin  = mean - sigma*factor;
		double xmax  = mean + sigma*factor;
		double xrange= xmax - xmin;
		int    nbins = (int) (xrange/0.005) + 1; // 5 ns/bin due to 200 MHz clock of TDC
		TH1D* hdT_zoom= new TH1D("hdT_zoom", "dT (us)", nbins, xmin, xmax );
    hdT_zoom->GetXaxis()->SetTitle( Form("t (J%03d) - t_ref (J%03d), 5ns/bin", febS_connID_B, febS_connID_A) );
		for( int i=0; i<vTimeA.size(); i++ )
	  {
	  	auto timeA = vTimeA.at(i);
	  	auto timeB = vTimeB.at(i);
	  	long long dt_sec  = timeB.GetSec()     - timeA.GetSec();
      long long dt_nano = timeB.GetNanoSec() - timeA.GetNanoSec();
      double dT = dt_sec + dt_nano * 1e-9;
	  	hdT_zoom->Fill( dT*1E6 );
		}

	  //hdT->GetXaxis()->SetTimeFormat("%H:%M:%S");
	  //hdT->Draw();
	  //hdT->GetXaxis()->SetNdivisions( 1010 );
	  //gPad->Modified();
	  //gPad->Update();
	  //c1->Print( pdfName.Data() );
	  
		hdT_zoom->GetXaxis()->SetTimeFormat("%H:%M:%S");
	  hdT_zoom->Draw();
	  hdT_zoom->GetXaxis()->SetNdivisions( 1010 );
	  gPad->Modified();
	  gPad->Update();
	  c1->Print( pdfName.Data() );
	}

	c1->Print( Form("%s]", pdfName.Data()) ); // close

	c1->Close();
	stg2->getTFile()->Close();

  return true;
}

