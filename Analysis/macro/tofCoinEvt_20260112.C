/// This code assumes that the number of 
void tofCoinEvt_20251121_KS(const char* finPath = "", const char* foutPathBase = "", int qdcmode = 1, const char* tdcCalibPath = "", const char* qdcCalibPath = "")
{
  if (TString(finPath).IsWhitespace() || TString(foutPathBase).IsWhitespace()) {
		printf("[ERR] Usage: .x tofCoinEvt_20251121.C(\"/abs/path/in.root\", \"/abs/path/out_base\")\n");
		return;
	}

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
}



