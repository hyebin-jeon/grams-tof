/// This code assumes that the number of 
void tofCoinEvt_20251121_KS(const char* finPath = "", const char* foutPathBase = "", int qdcmode = 1, const char* tdcCalibPath = "", const char* qdcCalibPath = "")
{
  if (TString(finPath).IsWhitespace() || TString(foutPathBase).IsWhitespace()) {
		printf("[ERR] Usage: .x tofCoinEvt_20251121.C(\"/abs/path/in.root\", \"/abs/path/out_base\")\n");
		return;
	}

	/// PETSYS setup
  double frequency = 200E6; // 200 MHz - doublecheck it
  //double frequency = 160E6; // 160 MHz - based on PETSYS's JINST paper
  double Tsec = 1/frequency; // second
	double Tns  = Tsec/pow(10,-9); // nsec

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

  TString pdfName  = Form("%s.pdf",  foutPathBase);
	TString rootName = Form("%s.root", foutPathBase);
	
	TFile* fin = new TFile( finPath, "read" );
	if( !fin->IsOpen() ) {
		printf("[ERR] FILE NOT FOUND: %s", finPath);
		return;
	}

	TOF_TreeData* tD = (TOF_TreeData*) fin->Get("data");
	if( !tD ) return;
	//tD->setBranchAddress();

 
	/// ROI channel list
	uint8_t febD_connID = 4;
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
	//tC->Write();

	std::vector<TOF_CoincidenceChannelInfo>* vBranch = nullptr;
	tC->SetBranchAddress( "coinEvt", &vBranch );


	/// Histograms 
	TH1D* hTDC[4]; // T branch's tdc
	TH1D* hTotC[4];  // using eCoarse
	TH1D* hTotF[4]; 
	TH1D* hQDC[4];
	TH1D* hQDC_Calib[4];
	TH1D* hTdiff[2]; // time diff between two sides of a paddle
	TH1D* hQratio[2]; // charge asym ratio between two sides of a paddle
	TH1D* hQCalratio[2]; // calibarated QDC's ratio
	TH1D* h_dT     = new TH1D("h_dT", "Time Diff between Paddles;Asym. time ratio;", 300, -15, 15 ); // nsec
	TH2D* hQvsQcal = new TH2D("hQvsQcal", "All Channels;QDC;Calibrated QDC", 200, 100, 300, 60, 0, 30 );
	TH2D* hTvsQraw = new TH2D("hTvsQraw", ";Time diff in clock;", 300, -3, 3, 100, -0.25, 0);
	TH2D* hTvsQcal = new TH2D("hTvsQcal", ";Time diff in clock;", 300, -3, 3, 100, -0.6, 0.6);

	for( int i=0; i<4; i++ )
	//for( auto chan: activeChannels )
	{
		double tdc_bin  = 200;
		double tdc_xmin = 0;
		double tdc_xmax = 0.128*1E12;
		//hTDC[i] = new TH1D( Form("hTdcT_ch%03d" , chan), "", tdc_bin, tdc_xmin, tdc_xmax ); // QDC max = 400 LSB
		hTDC[i] = new TH1D( Form("hTdcT_ch%03d" , activeChannels.at(i)), "", tdc_bin, tdc_xmin, tdc_xmax ); // QDC max = 400 LSB

		double tot_bin  = 100;
		double tot_xmin = 50;
		double tot_xmax = 70;
		hTotC[i] = new TH1D( Form("hTotC_ch%03d"  , activeChannels.at(i)), "", tot_bin, tot_xmin, tot_xmax ); // using eCoarse
		hTotF[i] = new TH1D( Form("hTotF_ch%03d"  , activeChannels.at(i)), "", tot_bin, tot_xmin, tot_xmax ); // using fine-tuned

		double qdc_bin  = 200;
		double qdc_xmin = qdcmode? 100:180;
		double qdc_xmax = qdcmode? 300:400;
		//hQDC[i] = new TH1D( Form("hEfine_ch%03d" , chan), "", qdc_bin, qdc_xmin, qdc_xmax ); // QDC max = 400 LSB
		hQDC[i] = new TH1D( Form("hEfine_ch%03d" , activeChannels.at(i)), "", qdc_bin, qdc_xmin, qdc_xmax ); // QDC max = 400 LSB
		
		double qdc_cal_bin  = qdcmode? 100:250;
		double qdc_cal_xmin = qdcmode? -5:-50;
		double qdc_cal_xmax = qdcmode? 35:200;
		//hQDC_Calib[i] = new TH1D( Form("hEfine_calib_ch%03d" , chan), "", qdc_cal_bin, qdc_cal_xmin, qdc_cal_xmax ); 
		hQDC_Calib[i] = new TH1D( Form("hEfine_calib_ch%03d" , activeChannels.at(i)), "", qdc_cal_bin, qdc_cal_xmin, qdc_cal_xmax ); 
	
		hTDC[i]->GetXaxis()->SetTitle( "TDC of T branch (t_begin)" );
		if( qdcmode ) {
			hQDC[i]      ->GetXaxis()->SetTitle( "QDC (=efine)" );
			hQDC_Calib[i]->GetXaxis()->SetTitle( "Calib QDC (=getEnergy)" );
		} else {
			hQDC[i]      ->GetXaxis()->SetTitle( "Time_end (=efine)" );
			hQDC_Calib[i]->GetXaxis()->SetTitle( "Time of charge integration" );
		}
	}
	for( int i=0; i<2; i++ )
	{
		int idx0 = i*2;
		int idx1 = idx0+1;
		
		hTdiff [i]    = new TH1D( Form("hTdiff_ch%03d-ch%03d"    , activeChannels.at(idx0), activeChannels.at(idx1) ), "", 100, 0, 0 ); 
		hQratio[i]    = new TH1D( Form("hQratio_ch%03d-ch%03d"   , activeChannels.at(idx0), activeChannels.at(idx1) ), "", 100, -1, 1 );
		hQCalratio[i] = new TH1D( Form("hQCalRatio_ch%03d-ch%03d", activeChannels.at(idx0), activeChannels.at(idx1) ), "", 100, -1, 1 );
		  
		hTdiff [i]->GetXaxis()->SetTitle( "time diff btw two ends of a paddle (in clocks)" );
		if( qdcmode ) {
		  hQratio[i]   ->GetXaxis()->SetTitle( "(q0-q1)/(q0+q1)" );
		  hQCalratio[i]->GetXaxis()->SetTitle( "(q0-q1)/(q0+q1) using calibrated QDC");
		} else {
		  hQratio[i]   ->GetXaxis()->SetTitle( "(t_end0-t_end1)/(t_end0+t_end1)" );
		  hQCalratio[i]->GetXaxis()->SetTitle( "(t_int0-t_int1)/(t_int0+t_int1), t_int=integ. time");
		}
	}
	
	if(qdcmode) {
	  hQvsQcal->GetXaxis()->SetTitle( "Raw QDC (=efine)" );
	  hQvsQcal->GetYaxis()->SetTitle( "Calib QDC (=getEnergy)" );
	  hTvsQraw->GetYaxis()->SetTitle( "Asym. ratio of QDC (efine)" );
	  hTvsQcal->GetYaxis()->SetTitle( "Asym. ratio of calib QDC (getEnergy)" );
	} else {
	  hQvsQcal->GetXaxis()->SetTitle( "Time_end (=efine)" );
	  hQvsQcal->GetYaxis()->SetTitle( "Time of charge integration" );
	  hTvsQraw->GetYaxis()->SetTitle( "Time_end (=efine)" );
	  hTvsQcal->GetYaxis()->SetTitle( "Asym. ratio of charge integ time" );
	}

	
	/// variables for a single paddle
	double pad_time[2]; 
	double pad_qdc[2];
	double pad_qdc_cal;

	uint32_t channel0 = activeChannels.at(0);

	//std::ofstream txtOut("tofCoinEvt_qdcCal.tsv");
	
	
	for( int i=0; i<tC->GetEntries(); i++ )
	{
		tC->GetEntry(i);

	  std::sort( vBranch->begin(), vBranch->end(), [](const TOF_CoincidenceChannelInfo& a, const TOF_CoincidenceChannelInfo& b) { return a.channelID < b.channelID; });

		double ttimeBegin[4];
		double qqdc[4];
		double qqdc_cal[4];
	  for( auto ele: *vBranch ) 
		{ 
	    auto frameID  = ele.frameID  ;
      auto channelID= (uint32_t) ele.channelID;
      auto tacID    = ele.tacID    ;
      auto tCoarse  = ele.tCoarse  ;
      auto eCoarse  = ele.eCoarse  ;
      auto tFine    = ele.tFine    ;
      auto eFine    = ele.eFine    ;
	    auto timeBegin= ele.timeBegin;
	    auto timeEnd  = ele.timeEnd  ;
	    double qdc_cal  = (double) ele.qdc_cal  ;

			auto chanA = channelID-channel0; 
			auto chanB = chanA%64;
			auto chanC = chanA/64;
			auto chanIdx = 2*chanC + chanB;
			
			hTDC[chanIdx]->Fill( timeBegin );
			hTotC[chanIdx]->Fill( timeEnd-timeBegin );
			hQDC[chanIdx]->Fill( eFine );
			hQDC_Calib[chanIdx]->Fill( qdc_cal );

			//txtOut<< qdc_cal << "\n";

			hQvsQcal->Fill( eFine, qdc_cal );

			ttimeBegin[chanIdx] = timeBegin;
			qqdc[chanIdx] = eFine;
			qqdc_cal[chanIdx] = qdc_cal;
			//cout << Form( "channelID: %03d, channel idx: %d, (A, B, C)= (%d, %d, %d)", channelID, chanIdx, chanA, chanB, chanC) << endl;
		}

		//cout << Form("channels: %03d, %03d, %03d, %03d", vBranch->at(0).channelID,  vBranch->at(1).channelID,  vBranch->at(2).channelID,  vBranch->at(3).channelID) << endl;

		double tdiff0 = ttimeBegin[0] - ttimeBegin[1];
		double tdiff1 = ttimeBegin[2] - ttimeBegin[3];
		double qratio0 = (qqdc[0] - qqdc[1])/(qqdc[0] + qqdc[1]);
		double qratio1 = (qqdc[2] - qqdc[3])/(qqdc[2] + qqdc[3]);
		double qCratio0 = (qqdc_cal[0] - qqdc_cal[1])/(qqdc_cal[0] + qqdc_cal[1]);
		double qCratio1 = (qqdc_cal[2] - qqdc_cal[3])/(qqdc_cal[2] + qqdc_cal[3]);

		hTdiff[0]->Fill( tdiff0 );
		hTdiff[1]->Fill( tdiff1 );

		hQratio[0]->Fill( qratio0 );
		hQratio[1]->Fill( qratio1 );
		
		hQCalratio[0]->Fill( qCratio0 );
		hQCalratio[1]->Fill( qCratio1 );
	
		hTvsQraw->Fill( tdiff0, qratio0 );
		hTvsQraw->Fill( tdiff1, qratio1 );
		hTvsQcal->Fill( tdiff0, qCratio0 );
		hTvsQcal->Fill( tdiff1, qCratio1 );

		double dT = ( (ttimeBegin[0]-ttimeBegin[2]) + (ttimeBegin[1]-ttimeBegin[3]) )/2.0* Tns;
		h_dT->Fill( dT );

		//if( i>100) break;
	}

	//txtOut.close();


	/// Individual channels' time and qdc
	TCanvas* c00_b = new TCanvas( "c00_b", "TDC_begin_allChannels", 800, 600 ); // t_begin
	TCanvas* c00_e = new TCanvas( "c00_e", "TDC_end_allChannels", 800, 600 ); // t_end
	TCanvas* c01 = new TCanvas( "c01", "QDC_allChannels", 800, 600 );
	TCanvas* c02 = new TCanvas( "c02", "QDC_calib_allChannels", 800, 600 );
	TCanvas* c03_T = new TCanvas( "c03_T", "by_Paddle" , 800, 600 );
	TCanvas* c03_Q = new TCanvas( "c03_Q", "by_Paddle" , 800, 600 );
	c00_b->Divide( 2,2 );
	c00_e->Divide( 2,2 );
	c01->Divide( 2,2 );
	c02->Divide( 2,2 );
	c03_T->Divide( 2,2 );
	c03_Q->Divide( 2,2 );
	gStyle->SetOptStat(111111);

	TText txt;
	txt.SetTextSize( 0.2 );
	txt.SetTextAlign(22);
	for( int i=0; i<4; i++ )
	{
	  c00_b->cd( i+1 );
		if( hTDC[i] ) hTDC[i]->Draw();
	  c00_e->cd( i+1 );
		if( hTotC[i] ) hTotC[i]->Draw();

	  c01->cd( i+1 );
		if( hQDC[i] ) hQDC[i]->Draw();
	  
		c02->cd( i+1 );
		if( hQDC_Calib[i] ) hQDC_Calib[i]->Draw();

		if( i<2 ) {
	    c03_T->cd( i+1 );
			if( hTdiff[i%2]  ) hTdiff[i%2]->Draw();
		  c03_Q->cd( i+1 );
			if( hQratio[i%2] ) hQratio[i%2]->Draw();

		}
		else {
	    c03_T->cd( i+1 );
			txt.DrawTextNDC( 0.5, 0.5, "N/A" );

		  c03_Q->cd( i+1 );
			if( hQCalratio[i%2] ) hQCalratio[i%2]->Draw();
		}
	}


	gStyle->SetOptFit(1111);
	TCanvas* c04 = new TCanvas( "c04", "Time Resolution of Two Paddles" );
	theFit->fitGauss( h_dT, 2.5 );
	auto fit = theFit->getFitFunction();
	h_dT->Draw();
	gPad->Modified();
	gPad->Update();
	auto ps = (TPaveStats*) gPad->GetPrimitive("stats");
	if( h_dT->GetMean() > 0 ) 
	  theAttrib->moveStatBoxNDC( ps, 0.12, 0.55, 0.40, 0.90 );
	else
	  theAttrib->moveStatBoxNDC( ps, 0.62, 0.55, 0.90, 0.90 );

	c00_b->Print( Form("%s(", pdfName.Data()) );
	c00_e->Print( pdfName.Data() );
	c01  ->Print( pdfName.Data() );
	c02  ->Print( pdfName.Data() );
	c03_T->Print( pdfName.Data() );
	c03_Q->Print( pdfName.Data() );
	c04  ->Print( pdfName.Data() );

	gStyle->SetOptStat(0);
	TCanvas* c05 = new TCanvas( "c05", "c05" );
	hQvsQcal->Draw("colz");
	
	TCanvas* c06 = new TCanvas( "c06", "c06" );
	hTvsQraw->Draw("colz");
	gPad->SetGrid();
	auto correlR = hTvsQraw->GetCorrelationFactor();
	theAttrib->drawTextNDC( 0.05, 12, kBlack, 0.15, 0.85, Form("correl= %.2f", correlR) );

	TCanvas* c07 = new TCanvas( "c07", "c07" );
	hTvsQcal->Draw("colz");
	gPad->SetGrid();
	auto correl = hTvsQcal->GetCorrelationFactor();
	theAttrib->drawTextNDC( 0.05, 12, kBlack, 0.15, 0.85, Form("correl= %.2f", correl) );

	c05->Print( pdfName.Data() );
	c06->Print( pdfName.Data() );
	c07->Print( Form("%s)", pdfName.Data()) );


	fout->Write();
	fout->Close();

	cout << "=================================" << endl;
	cout << "ouput: " << pdfName.Data() << endl;
	cout << "=================================" << endl;

}



