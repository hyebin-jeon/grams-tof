/// Last updated on 20260129


void tofCoinEvt_20260125_verKS(
	const char* finPath      = "../data/20260122/run_2026-01-22_22-22-37.717Z.stg1.root", 
	const char* foutPathBase = "./output/tofCoinEvt_20260125_verKS", int qdcmode = 1, 
	const char* tdcCalibPath = "../calibration/20260122/tdc_calibration_2026-01-22_21-03-10.916Z.tsv", 
	const char* qdcCalibPath = "../calibration/20260122/qdc_calibration_2026-01-22_21-07-30.19Z.tsv"
	)
{

  if (TString(finPath).IsWhitespace() || TString(foutPathBase).IsWhitespace()) {
		printf("[ERR] Usage: .x tofCoinEvt_20251121.C(\"/abs/path/in.root\", \"/abs/path/out_base\")\n");
		return;
	}

  /// warm up ///
  //gROOT->SetBatch(kTRUE);
  //TF1 *f_warmup = new TF1("warmup", "gaus", 0, 1);
  //delete f_warmup;
  //TCanvas *c_warmup = new TCanvas("c_warmup", "warmup");
  //c_warmup->Print("warmup_init.pdf");
  //delete c_warmup;
  //gSystem->Unlink("warmup_init.pdf"); // Remove the dummy file


	/// Class setup
	TOF_CoincidenceEvents* theCoin = new TOF_CoincidenceEvents();
  auto theChanConv = TOF_ChannelConversion::getInstance();
	auto theFit      = TOF_Fitting::getInstance();
  auto theAttrib   = TOF_Attributes::getInstance();
  auto theCalib    = TOF_TdcQdcCalibration::getInstance();
 
  // Logic for Calibration Files
  TString tdcPath = tdcCalibPath;
  TString qdcPath = qdcCalibPath;
  //TString glibDir = gSystem->Getenv("GLIB");

  //if (tdcPath.IsWhitespace()) {
  //  tdcPath = Form("%s/config/tdc_calibration.tsv", glibDir.Data());
  //}
  //if (qdcPath.IsWhitespace()) {
  //  qdcPath = Form("%s/config/qdc_calibration.tsv", glibDir.Data());
  //}

  // Load specified or default calibration files
  printf("[INFO] Loading TDC calibration: %s\n", tdcPath.Data());
  printf("[INFO] Loading QDC calibration: %s\n", qdcPath.Data());
  theCalib->readTdcCalib(tdcPath);
  theCalib->readQdcCalib(qdcPath);

	cout << foutPathBase << endl;

  TString pdfName  = Form("%s.pdf",  foutPathBase);
	TString rootName = Form("%s.root", foutPathBase);
	

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


	/// Convert Stg1 to Stg2 to add timestamp ///
	cout << "-------------------------------" << endl;
	cout << "Convert Stg1 to Stg2 to add timestamp" << endl;
	cout << "-------------------------------" << endl;
	auto theConvStg = TOF_ConvertStg1toStg2::getInstance();
	theConvStg->convertStg1ToStg2( finPath ); // default
	//theConvStg->convertStg1ToStg2( finPath, "output/timestamp.stg2.root" ); // specify output file name

	/// Find coincidence events ///
	cout << "-------------------------------" << endl;
	cout << "Coincidence events" << endl;
	cout << "-------------------------------" << endl;
	auto fpathStg2 =  theConvStg->getStg2()->getFilePath();
	theCoin->setInputPathStg2( Form("%s", fpathStg2.c_str()) );

	/// class setup
	theCoin->setActiveChannels( activeChannels );
	
	TFile *fout = new TFile(rootName, "recreate" );
	//theCoin->setQdcCalibMethod( TOF_QdcCalibMethod::fGetEnergy );
	auto tC = theCoin->getCoincidenceEventsTree();
	tC->Print();
	tC->Write();

	theCoin->generateHistoForQA();
	theCoin->fHisto_dT     ->Write();
	theCoin->fHisto_NbOfEvt->Write();
	theCoin->fHisto_TvsQcal->Write();

	cout << "=================================" << endl;
	cout << "ouput: " << fout->GetName() << endl;
	cout << "=================================" << endl;

	fout->Close();

}



