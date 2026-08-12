#include "process_tofQA_iridium.h"

#include <TH1F.h>
#include <TCanvas.h>
#include <TString.h>
#include <TStyle.h>
#include <TFile.h>
#include <TOF_Constants.h>
#include <TOF_Attributes.h>
#include <TOF_ChannelConversion.h>
#include <TOF_PaddleChannelMap.h>
#include <TOF_TreeDataStg2.h>

bool runTofQA_Iridium( const std::string& inputFile,
                       const std::string& outputBase,
											 const std::string& asicListFile
											 //const int runTimeSec_
											 )
{
	/// output naming
  const char* inputFile_c = inputFile.c_str();	
	
	/// channel conversion class
	auto theAttrib = TOF_Attributes::getInstance();

	/// channel conversion class
	auto theChanConv = TOF_ChannelConversion::getInstance();
	auto thePaddle   = TOF_PaddleChannelMap::getInstance();
	auto theAsicList = TOF_ActiveAsicList::getInstance();
	//theAsicList->readActiveAsicList();
	auto activeConnIds_D = theAsicList->getActiveConnIdOnFebD();
	
	const int nconn = fNbOfFebS; //2; // number of connected FEB-S
	const int nsyst  = 2; // number of system TTOF, MTOF, MPD
	const int npad[3] = { fTTOF_NbChannels, fMTOF_NbChannels }; //, fMPD_NbChannels };

	//if( activeConnIds_D.size() != nconn ) {
	//	std::cerr << "[WARN] Number of Active Asic ConnectorID List != 2" << std::endl;
	//}

	/// input stg2
	TOF_TreeDataStg2* stg2 = new TOF_TreeDataStg2();
	stg2->setInputPath( inputFile_c );
	stg2->setBranchAddress();

	/// x-axis time range
	const int binW = 10; // in sec
	stg2->getEntry(0);
	TTimeStamp t_begin = stg2->getTimeStampCPU();
	stg2->getEntry( stg2->getEntries()-1 );
	TTimeStamp t_end = stg2->getTimeStampCPU();
	int tmin = t_begin.GetSec();
	int dur_nano = t_end.GetNanoSec() - t_begin.GetNanoSec();
	int dur  = t_end.GetSec() - t_begin.GetSec();
	if( dur_nano < 0 ) dur = dur -1;
	else if( dur_nano > 0 ) dur = dur + 1;
	int tmax = dur%binW==0? t_end.GetSec() : t_end.GetSec()+ binW;

	const double runTimeSec = (t_end.GetSec() + (double)t_end.GetNanoSec()*1E-9) - (t_begin.GetSec() + (double)t_begin.GetNanoSec()*1E-9);;
	std::cout << Form("[INFO] Run Duration: %10.2f (sec) = %9.2f (min) = %8.2f (hours)", runTimeSec, runTimeSec/60., runTimeSec/3600.) << std::endl;

	/// histograms
	int timeRange = tmax - tmin;
	int timeBinNb  = timeRange%10? timeRange/10+1 : timeRange/10;
	TH1F* hEvtPaddle [nconn];
	TH1F* hEvtCPUTime[nconn];
	for( int i=0; i<nconn; i++ )
	{
	  hEvtPaddle[i] = new TH1F(Form("hEvtPaddle%d",i), Form("PaddleID vs. Event rate (FebD_%02d)", activeConnIds_D[i]), 120, -10, 50 );
		hEvtPaddle[i]->GetXaxis()->SetTitle("Paddle ID (NS)");
		hEvtPaddle[i]->GetYaxis()->SetTitle("Event rate (Hz)");

	  hEvtCPUTime[i] = new TH1F(Form("hEvtCPUTime%d",i), Form("CPU time vs. Event rate (FebD_%02d)", activeConnIds_D[i]), timeBinNb, tmin, tmax); // 1800 sec = 40 min, 1 bin per 10 sec
		hEvtCPUTime[i]->GetXaxis()->SetTitle("CPU time (UTC), 10 sec/bin");
		hEvtCPUTime[i]->GetYaxis()->SetTitle("Event rate (Hz)");
	}

	/// fill the histo
	for( int i=0; i<stg2->getEntries(); i++)
	{
		stg2->getEntry(i);

		//auto ts_pps   = stg2->getTimeStampPPS();
		auto ts_cpu   = stg2->getTimeStampCPU();
		auto connID_D = stg2->getConnID_FebD();
		auto connID_S = stg2->getConnID_FebS();
		auto paddleIdx = stg2->getPaddleIdx();
		
		int febS_idx= thePaddle->getFebSIdx( connID_D );
		if( febS_idx<0 ) continue;

		int systIdx   = (int) thePaddle->getSystemIdx  ( paddleIdx );
		int paddLocID = (int) thePaddle->getPaddleLocId( paddleIdx );
		int sipmLocID = (int) thePaddle->getSipmLocId  ( paddleIdx );

		double syst_offset = systIdx * 30.;
		double sipmLocID_d = (sipmLocID%2)* 0.5;
		double paddle_bin = syst_offset + paddLocID + sipmLocID_d;

		/// exceptioanl cases
		/// trigger channel --> paddle_bin = -3
		/// pps     channel --> paddle_bin = -4
		/// test paddles    --> paddle_bin = -5 and -5.5 for the test paddle on UTOF, -6 and -6.5 for the test paddle on MTOF
		if     ( thePaddle->isTriggerChannel( paddleIdx )==true ) paddle_bin = -1*systIdx;
		else if( thePaddle->isPpsChannel    ( paddleIdx )==true ) paddle_bin = -1*systIdx;
		else if( thePaddle->isTestPaddle    ( paddleIdx )==true ) paddle_bin = -1*(systIdx + paddLocID + (sipmLocID%2)*0.5);

		//if( paddle_bin<0 )
		//	std::cout << Form("connID_D: %3u, connID_S: %03u, paddle Idx: 0x%04X --> system: %u, paddle locID: %2u, sipm locID: %u --> paddle_bin = %2.1f", connID_D, connID_S, paddleIdx, systIdx, paddLocID, sipmLocID, paddle_bin) << std::endl;

		hEvtPaddle [febS_idx]->Fill( paddle_bin, 1./ runTimeSec );
		if( paddle_bin>0 ) hEvtCPUTime[febS_idx]->Fill( ts_cpu.AsDouble(), 1./(double)binW );
	}

	/// output file naming
	std::string fin_root = stg2->getFileName();
	std::string fin_dir  = std::filesystem::path(inputFile_c).parent_path();
	auto        fin_idx  = ((TString) fin_root).Index(".root");
	TString     fin_name = ((TString) fin_root)(0, fin_idx);

	std::string fout_dir = outputBase.empty()==true? fin_dir :  outputBase;
	std::string fout_suffix = "iridiumQA";
	const char* fout_name = Form("%s/%s.%s", fout_dir.c_str(), fin_name.Data(), fout_suffix.c_str());
	const char* fout_root = Form("%s.root", fout_name);
	const char* fout_pdf  = Form("%s.pdf" , fout_name);

	TFile* fout = new TFile( fout_root, "recreate");

	TCanvas* canv0 = new TCanvas("canv0", "canv0"); //, 1100, 500);
	canv0->Print( Form("%s[", fout_pdf) ); // open 
	canv0->Divide(nconn,2,0.005,0.005);

	TText txt;
	txt.SetTextColorAlpha( kBlack, 0.5 );
	txt.SetTextSize(0.1);
	txt.SetTextAlign(11); // left bottom
	TText txt_angled;
	txt_angled.SetTextColorAlpha( kBlack, 0.5 );
	txt_angled.SetTextSize(0.05);
	txt_angled.SetTextAlign(31); // left bottom
	txt_angled.SetTextAngle(270);

	gStyle->SetOptStat(111111);
	for( int j=0; j<nconn; j++ )
	{
	  /// scale the histo to make y axis = event rate
		//hEvtPaddle[j]->Scale( 1./(double) runTimeSec );

		theAttrib->attribHist( hEvtPaddle[j] );
		theAttrib->attribHist( hEvtCPUTime[j] );
		hEvtCPUTime[j]->GetXaxis()->SetTimeDisplay(1);
	  hEvtCPUTime[j]->GetXaxis()->SetTimeFormat ("%m/%d %H:%M");
	  hEvtCPUTime[j]->GetXaxis()->SetTimeOffset(0, "gmt");

	  canv0->cd(j*2+1);
	  hEvtPaddle[j]->Draw("hist");
	  hEvtPaddle[j]->GetXaxis()->SetNdivisions( 520 );
		txt.DrawText(  1, 0, "UTOF" ); 
		txt.DrawText( 31, 0, "MTOF" ); 
		txt_angled.DrawText( -3, 0, "TRG" );
		txt_angled.DrawText( -4, 0, "PPS" );
		txt_angled.DrawText( -5.5, 0, "Spare on UTOF" );
		txt_angled.DrawText( -6.5, 0, "Spare on MTOF" );
		
	  canv0->cd(j*2+2);
	  hEvtCPUTime[j]->Draw("hist");

		gPad->Modified();
		gPad->Update();
		auto stat = (TPaveStats *) gPad->GetPrimitive("stats");
		theAttrib->moveStatBoxNDC( stat, 0.75, 0.15, 0.95, 0.40 );
		

		hEvtPaddle[j]->Write();
		hEvtCPUTime[j]->Write();
	}

	canv0->Print( fout_pdf );
	canv0->Print( Form("%s]", fout_pdf) ); // close

	fout->Close();

	std::cout << "[INFO] Output (root): " << fout->GetName() << std::endl;
	std::cout << "[INFO] Output (pdf ): " << fout_pdf << std::endl;

	return true;

}
