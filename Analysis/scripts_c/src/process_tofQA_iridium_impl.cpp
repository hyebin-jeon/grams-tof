#include "process_tofQA_iridium.h"

#include <TH1F.h>
#include <TCanvas.h>
#include <TString.h>
#include <TStyle.h>
#include <TFile.h>
#include <TParameter.h>
#include <TOF_Constants.h>
#include <TOF_Attributes.h>
#include <TOF_ChannelConversion.h>
#include <TOF_PaddleChannelMap.h>
#include <TOF_TreeDataStg2.h>

bool runTofQA_Iridium( const std::string& inputFile,
                       const std::string& outputBase
		       //const std::string& asicListFile // unused.. 
		       //const int runTimeSec_
			 )
{
	/// output naming
  const char* inputFile_c = inputFile.c_str();	
	TString name_root = std::filesystem::path(inputFile_c).filename().c_str();
	if( !name_root.EndsWith(".stg2.root") ) {
		std::cerr<< "[ERR] Wrong Input File. Provide *.stg2.root" << std::endl;
		return false;
	}
	
	/// channel conversion class
	auto theAttrib = TOF_Attributes::getInstance();
	auto theChanConv = TOF_ChannelConversion::getInstance();
	auto theAsicList = TOF_ActiveAsicList::getInstance();
	auto thePaddle   = TOF_PaddleChannelMap::getInstance();

		
	const int nconn = fNbOfFebS; // = 1; // number of connected FEB-S
	const int nsyst  = 2; // number of system TTOF, MTOF, MPD
	const int npad[3] = { fTTOF_NbChannels, fMTOF_NbChannels }; //, fMPD_NbChannels };
	  

  /// get PPS and TRIG channel IDs
  auto     connIDs_A     = thePaddle->getConnectorIDs_PPS();
	int      febD_connID_A = connIDs_A.first;
	int      febS_connID_A = connIDs_A.second;
  uint32_t chA           = theChanConv->getAbsoluteChannelID( febD_connID_A, febS_connID_A );

	/// input stg2
	TOF_TreeDataStg2* stg2 = new TOF_TreeDataStg2();
	stg2->setInputPath( inputFile_c );
	if( stg2->getEntries() == 0 ) {
		std::cerr<< "[ERR] No Entries in Stg2. Terminate." << std::endl;
		return 0;
	}
	stg2->setBranchStatus("*",1);
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

	
	/// protection for the old data
	auto connD = stg2->getConnID_FebD();
	//auto connS = stg2->getConnID_FebS();
	auto activeConnIds_D = theAsicList->getActiveConnIdOnFebD();
	if( activeConnIds_D[0] != connD ) {
		std::cout << "[WARN] FEB/D connector ID saved in TOF_ActiveAsicList != Actual connector ID from Stg2 data. --> Force to reset it to be the actual connector ID" << std::endl;
	  theAsicList->setActiveConnIdOnFebD( connD, -1 ); // -1 is dummy connector ID for the sencod FEB/S
		activeConnIds_D = theAsicList->getActiveConnIdOnFebD();
	}

	/// PPS graph
	TGraph* gPPS= new TGraph(); 
	gPPS->SetNameTitle("gPPS", "PPS;PPS counts;CPU Time (UTC)");
	theAttrib->attribGraph( gPPS );
	gPPS->SetMarkerSize( 0.5 );
	int pps_count = 0;

	/// Event rate histograms
	int timeRange = tmax - tmin;
	int timeBinNb  = timeRange%10? timeRange/10+1 : timeRange/10;
	
	TH1F* hEvtCPUTime = new TH1F("hEvtCPUTime", Form("CPU time vs. Event rate (FebD_%02d)", activeConnIds_D[0]), timeBinNb, tmin, tmax); // 1800 sec = 40 min, 1 bin per 10 sec
	hEvtCPUTime->GetXaxis()->SetTitle("CPU time (UTC), 10 sec/bin");
	hEvtCPUTime->GetYaxis()->SetTitle("Event rate (Hz)");

	TH1F* hEvtPaddle_UTOF = new TH1F("hEvtPaddle_UTOF", Form("UTOF Event rate (FebD_%02d)", activeConnIds_D[0]), 56, -2, 26 );
	hEvtPaddle_UTOF->GetXaxis()->SetTitle("UTOF Paddle ID (2 ch/paddle)");
	hEvtPaddle_UTOF->GetYaxis()->SetTitle("Event rate (Hz)");
	//hEvtPaddle_UTOF->GetXaxis()->SetNdivisions( 520 );
	
	TH1F* hEvtPaddle_MTOF = new TH1F("hEvtPaddle_MTOF", Form("MTOF Event rate (FebD_%02d)", activeConnIds_D[0]), 40, -2, 18 );
	hEvtPaddle_MTOF->GetXaxis()->SetTitle("MTOF Paddle ID (2 ch/paddle)");
	hEvtPaddle_MTOF->GetYaxis()->SetTitle("Event rate (Hz)");
	//hEvtPaddle_MTOF->GetXaxis()->SetNdivisions( 520 );

	//float mpd_binW = 1./6;
	//float mpd_x0 = 1-mpd_binW;
	//float mpd_x1 = 3+mpd_binW;
	//float mpd_bin = (mpd_x1 - mpd_x0 )/mpd_binW;
	TH1F* hEvtPaddle_MPD = new TH1F("hEvtPaddle_MPD", Form("MPD Event rate (FebD_%02d)", activeConnIds_D[0]), 4*6, 0, 4 );
	hEvtPaddle_MPD->GetXaxis()->SetTitle("MPD Paddle ID (6 ch/paddle)");
	hEvtPaddle_MPD->GetYaxis()->SetTitle("Event rate (Hz)");

	//double evtRate_PPS{0}, evtRate_TRG{0};
  TParameter<double> evtRate_PPS( "evtRate_PPS", 0 );
	TParameter<double> evtRate_TRG( "evtRate_TRG", 0 );



	/// fill the histo
	for( int i=0; i<stg2->getEntries(); i++)
	{
		stg2->getEntry(i);
		
		uint32_t channelID = stg2->getChannelID();
		auto ts_cpu   = stg2->getTimeStampCPU();
		auto connID_D = stg2->getConnID_FebD();
		auto connID_S = stg2->getConnID_FebS();
		auto paddleIdx = stg2->getPaddleIdx();

		/// pps
		if( channelID == chA ){  
			gPPS->AddPoint( pps_count, ts_cpu );
			pps_count++;
		}

		int febS_idx= thePaddle->getFebSIdx( connID_D );
		if( febS_idx<0 ) continue;
		if( febS_idx!=0 ) continue; // use only first idx for the upcoming flight (2026)

		int systIdx   = (int) thePaddle->getSystemIdx  ( paddleIdx );
		int paddLocID = (int) thePaddle->getPaddleLocId( paddleIdx );
		int sipmLocID = (int) thePaddle->getSipmLocId  ( paddleIdx );

		double syst_offset = systIdx * 30.;
		double sipmLocID_d = (sipmLocID%2)* 0.5;
		double paddle_bin = paddLocID + sipmLocID_d;
		
		/// exceptioanl cases
		/// trigger channel --> paddle_bin = -3
		/// pps     channel --> paddle_bin = -4
		/// test paddles    --> paddle_bin = -5 and -5.5 for the test paddle on UTOF, -6 and -6.5 for the test paddle on MTOF
		if     ( thePaddle->isTriggerChannel( paddleIdx )==true ) { evtRate_TRG.SetVal( evtRate_TRG.GetVal() + 1./runTimeSec ); continue; } 
		else if( thePaddle->isPpsChannel    ( paddleIdx )==true ) { evtRate_PPS.SetVal( evtRate_PPS.GetVal() + 1./runTimeSec ); continue; } 

		if( paddle_bin>0 ) hEvtCPUTime->Fill( ts_cpu.AsDouble(), 1./(double)binW );

		if     ( systIdx == eSystem::fUTOF ) hEvtPaddle_UTOF->Fill( paddle_bin, 1./ runTimeSec );
		else if( systIdx == eSystem::fMTOF ) hEvtPaddle_MTOF->Fill( paddle_bin, 1./ runTimeSec );
		else if( systIdx == eSystem::fMPD  ) {
		  double mpd_sipmLocID_d = sipmLocID* 1./6.;
		  double mpd_paddle_bin  = paddLocID + mpd_sipmLocID_d;
			hEvtPaddle_MPD->Fill( mpd_paddle_bin, 1./ runTimeSec  );
		}
		else if( systIdx == eSystem::fTest ) {
			paddle_bin = -1 + sipmLocID*0.5;
			if     ( paddLocID == eSystem::fUTOF ) hEvtPaddle_UTOF->Fill( paddle_bin, 1./ runTimeSec );
			else if( paddLocID == eSystem::fMTOF ) hEvtPaddle_MTOF->Fill( paddle_bin, 1./ runTimeSec );
		}

		//if( paddle_bin<0 )
		//	std::cout << Form("connID_D: %3u, connID_S: %03u, paddle Idx: 0x%04X --> system: %u, paddle locID: %2u, sipm locID: %u --> paddle_bin = %2.1f", connID_D, connID_S, paddleIdx, systIdx, paddLocID, sipmLocID, paddle_bin) << std::endl;
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
	canv0->Divide(1,3,0.005,0.005);

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
	  
	/// scale the histo to make y axis = event rate
	//hEvtPaddle->Scale( 1./(double) runTimeSec );

	theAttrib->attribHist( hEvtCPUTime );
	theAttrib->attribHist( hEvtPaddle_UTOF);
	theAttrib->attribHist( hEvtPaddle_MTOF);
	theAttrib->attribHist( hEvtPaddle_MPD );

	hEvtCPUTime->GetXaxis()->SetTimeDisplay(1);
	hEvtCPUTime->GetXaxis()->SetTimeFormat ("%m/%d %H:%M");
	hEvtCPUTime->GetXaxis()->SetTimeOffset(0, "gmt");
	
	canv0->cd(1);
	TPad* padUT = new TPad("padUT", "UTOF", 0.005, 0.005, 0.400, 0.995 );
	TPad* padMT = new TPad("padMT", "MTOF", 0.405, 0.005, 0.750, 0.995 );
	TPad* padMP = new TPad("padMP", "MPD" , 0.755, 0.005, 0.995, 0.995 );
	padUT->Draw();
	padMT->Draw();
	padMP->Draw();

	padUT->cd();
	hEvtPaddle_UTOF->Draw("hist");
	gPad->SetGridx();
	padMT->cd();
	hEvtPaddle_MTOF->Draw("hist");
	gPad->SetGridx();
	padMP->cd();
	hEvtPaddle_MPD->GetXaxis()->SetRangeUser( 1-1./6, 3+1./6 );
	hEvtPaddle_MPD ->Draw("hist");
	gPad->SetGridx();


	//txt.DrawText(  1, 0, "UTOF" ); 
	//txt.DrawText( 31, 0, "MTOF" ); 
	//txt_angled.DrawText( -3, 0, "TRG" );
	//txt_angled.DrawText( -4, 0, "PPS" );
	txt_angled.DrawText( -1, 0, "Spare" );
	txt_angled.DrawText( -1, 0, "Spare" );
		
	canv0->cd(2);
	gPad->SetLeftMargin(0.06);
	gPad->SetRightMargin(0.06);
	hEvtCPUTime->Draw("hist");
	gPad->Modified();
	gPad->Update();
	auto stat = (TPaveStats *) gPad->GetPrimitive("stats");
	theAttrib->moveStatBoxNDC( stat, 0.75, 0.15, 0.95, 0.40 );
		
	canv0->cd(3);
	if( gPPS->GetN() == 0 ) {
		txt.SetTextAlign(22);
		txt.DrawText(0.5, 0.5, Form("No PPS signal on FEB/D conn.%d and FEB/S conn.%d", febD_connID_A, febS_connID_A) );
	}
  else {
	  gPad->SetLeftMargin(0.06);
	  gPad->SetRightMargin(0.06);
	  gPPS->Draw("apl");
	  gPPS->GetYaxis()->SetTitleOffset( 0.2 );
	  gPPS->GetYaxis()->SetTimeDisplay(1);
	  gPPS->GetYaxis()->SetTimeFormat ("%m/%d %H:%M:%S");
	  gPPS->GetYaxis()->SetTimeOffset(0, "gmt" );
	}

  //canv0->cd(4);
	//hEvtPaddle_MPD->Draw("hist");
	//hEvtPaddle_MPD->GetXaxis()->SetNdivisions( 520 );
	//gPad->SetGridy();
	////gPad->SetGridx();
	
canv0->Print( fout_pdf );
	canv0->Print( Form("%s]", fout_pdf) ); // close


	hEvtCPUTime->Write();
	hEvtPaddle_UTOF->Write();
	hEvtPaddle_MTOF->Write();
	hEvtPaddle_MPD ->Write();
	gPPS->Write();
	evtRate_TRG.Write();
	evtRate_PPS.Write();

	//fout>WriteObjectAny(&evtRate_TRG, "double", "evtRate_TRG");
	//fout>WriteObjectAny(&evtRate_PPS, "double", "evtRate_PPS");

	fout->Close();

	std::cout << "[INFO] Output (root): " << fout->GetName() << std::endl;
	std::cout << "[INFO] Output (pdf ): " << fout_pdf << std::endl;

	return true;

}
