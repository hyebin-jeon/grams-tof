/// v3

#include "TOF_CoincidenceEvents.h"

ClassImp( TOF_CoincidenceEvents );
		
void TOF_CoincidenceEvents::setActiveChannels( std::vector<uint32_t> chanList )
{
	fActiveChannelList.clear();

	if( chanList.size() == 0 ) {
		std::cout<< "TOF_QdcCalibMethod::setActiveChannels(). A given channel list is empty." << std::endl;
	}

	std::sort( chanList.begin(), chanList.end() );

	fActiveChannelList = chanList;

	//for( auto chan: fActiveChannelList )
	//	std::cout << Form("[TOF_CoincidenceEvents::setActiveChannels] active channel: %03d", chan) << std::endl;

	return;
}

int TOF_CoincidenceEvents::setTreeData( TOF_TreeData* tr )
{
	if( !tr ) {
		std::cout << "[ERR] TOF_CoincidenceEvents::setTreeDat( TOF_TreeData* tr ), Given 'tr' NOT FOUND" << std::endl;
		return -1;
	}
	tr->SetBranchStatus("*",1);

	fTree= tr;
	fTree->setBranchAddress();

	return 1;
}

std::vector<std::vector<TOF_CoincidenceChannelInfo>> TOF_CoincidenceEvents::getCoincidenceEvents()
{
	if( !fTree ) {
		std::cout << "[ERR] TOF_QdcCalibMethod::getCoincidenceEvents(). fTree is NULL. Exit." << std::endl;
		return vTree;
	}

	if( fTree->GetEntries() == 0 ) {
		std::cout << "[ERR] TOF_QdcCalibMethod::getCoincidenceEvents(). fTree's entries = 0. Exit." << std::endl;
		return vTree;
	}
	fTree->setBranchAddress(); // duplicate

	vTree.clear();

	//std::cout << "ok1" << std::endl;

	std::cout << "ok1.2" << std::endl;
	std::cout << "Address fTree: " << fTree << std::endl;

	fTree->GetEntry(0);
	long long frameID0= fTree->getFrameID(); 
	long long lastTime= CLOCKS_IN_A_FRAME * frameID0 + fTree->getTCoarse(); 	
	long long currTime= -1; // 64-bit signed int

	TOF_CoincidenceChannelInfo channelInfo{};
	TOF_CoincidenceChannelInfo channelBrElement{};
	std::map< uint32_t, TOF_CoincidenceChannelInfo> vChannelData;
	std::map< uint32_t, bool > vHitChannel;

	const int chanSize = fActiveChannelList.size();
	
	std::cout << "ok2. fTree->GetEntries()= " << fTree->GetEntries() << std::endl;
	//fTree->Print();
	  
	std::vector<TOF_CoincidenceChannelInfo> vBranchTemp;
	vBranchTemp.clear();

	for( int i=0; i<fTree->GetEntries(); i++ )
	{
		fTree->GetEntry(i);

		auto frameID      = fTree->getFrameID();
		auto tCoarse      = fTree->getTCoarse();
		auto absChannelID = fTree->getChannelID(); // absolute channel ID

		channelInfo.frameID   = frameID;
		channelInfo.channelID = absChannelID;
		channelInfo.tacID   = fTree->getTacID();
		channelInfo.tCoarse = fTree->getTCoarse();
		channelInfo.eCoarse = fTree->getECoarse();
		channelInfo.tFine   = fTree->getTFine()  ;
		channelInfo.eFine   = fTree->getEFine()  ;

		long long time_trigCh;
		if( absChannelID == fTrigChannelID ) time_trigCh = CLOCKS_IN_A_FRAME * frameID + tCoarse;

		/// current time in clock
		currTime = CLOCKS_IN_A_FRAME * frameID + tCoarse;
		
			
		/// coincidence events within 'coincidenceT' clocks
		if( fabs(currTime - lastTime) > fCoinTimeWindow) 
		{
			vChannelData.clear();
			vHitChannel.clear();
		}

		for( auto activeChanID: fActiveChannelList )
		{
		  if( absChannelID == activeChanID )
		  {
				vHitChannel [absChannelID] = true;
		    vChannelData[absChannelID] = channelInfo;
			}
		}

		if( vChannelData.size() == 1 ) lastTime = currTime;
		
		bool good=1;
		for( auto activeChanID: fActiveChannelList )
		{
			good &= vHitChannel[ activeChanID ];
		}

		if( i==0 )
		std::cout << Form("[%03d] good: %d, currTime: %10lld, frameID= %10lld, tCoarse= %10hu, tFine= %10hu, channelID= %3u", i, good, currTime, frameID, channelInfo.tCoarse, channelInfo.tFine, channelInfo.channelID ) << std::endl;

		if( vChannelData.size() == fActiveChannelList.size() && good )
		{
			vBranchTemp.clear();

			//for( const auto [chan, chanData]: vChannelData )
				//std::cout << Form("coincidence channels: %03d, idx = %03d", chanData.channelID, chan ) << std::endl;

		  for( auto activeChanID: fActiveChannelList )
			{
				auto hit_channelID = activeChanID;
		    auto hit_frameID   = vChannelData[activeChanID].frameID;
		    auto hit_tacID     = vChannelData[activeChanID].tacID  ;
		    auto hit_tCoarse   = vChannelData[activeChanID].tCoarse;
		    auto hit_eCoarse   = vChannelData[activeChanID].eCoarse;
		    auto hit_tFine     = vChannelData[activeChanID].tFine  ;
		    auto hit_eFine     = vChannelData[activeChanID].eFine  ;

				//std::cout << Form("[%03d] [coincidence] channelID: %03d, frameID= %10lld, tacID= %d, tCoarse= %10hu, tFine= %10hu, eCoarse= %10hu, eFine= %10hu,", i, hit_channelID, hit_frameID, hit_tacID, hit_tCoarse, hit_tFine, hit_eCoarse, hit_eFine ) << std::endl;

				auto hit_timeBegin = TOF_TdcQdcCalibration::getInstance()->getCalibratedTime( TOF_Branch::fBranchT, hit_channelID, hit_tacID, hit_frameID, hit_tCoarse, hit_tFine );
				//auto hit_timeBegin = theCalib->getCalibratedTime( TOF_Branch::fBranchT, hit_channelID, hit_tacID, hit_frameID, hit_tCoarse, hit_tFine );
				
				//std::cout << Form("[%03d] [coincidence] channelID: %03d, t_begin: %8.2f", i, hit_channelID, hit_timeBegin ) << std::endl;

				//auto ecoarse = channelBrElement.eCoarse;
				auto ecoarse = hit_eCoarse;
	      if((hit_eCoarse - hit_tCoarse) < -256) ecoarse += 1024;
				auto hit_timeEnd = double((frameID*1024+ ecoarse));
				
				//std::cout << Form("[%03d] [coincidence] channelID: %03d, t_begin: %8.2f, t_end: %8.2f", i, hit_channelID, hit_timeBegin, hit_timeEnd ) << std::endl;

				double hit_qdc_cal = 0;
				if( fQdcCalibMethod == TOF_QdcCalibMethod::fGetEnergy )
					hit_qdc_cal = TOF_TdcQdcCalibration::getInstance()->getEnergy( hit_channelID, hit_tacID, hit_frameID, hit_eCoarse, hit_eFine, hit_timeBegin  );
					//hit_qdc_cal = theCalib->getEnergy( hit_channelID, hit_tacID, hit_frameID, hit_eCoarse, hit_eFine, hit_timeBegin  );
				else if( fQdcCalibMethod == TOF_QdcCalibMethod::fLinear ) 
					hit_qdc_cal = 0; ////////////////////////////////
				else if( fQdcCalibMethod == TOF_QdcCalibMethod::fTiming ) 
					hit_qdc_cal =	TOF_TdcQdcCalibration::getInstance()->getCalibratedQDC( hit_channelID, hit_tacID, hit_frameID, hit_eCoarse, hit_eFine, hit_tCoarse, hit_timeBegin  );
					//hit_qdc_cal =	theCalib->getCalibratedQDC( hit_channelID, hit_tacID, hit_frameID, hit_eCoarse, hit_eFine, hit_tCoarse, hit_timeBegin  );
				else hit_qdc_cal = 0;
				
				channelBrElement.channelID = hit_channelID;
		    channelBrElement.frameID   = hit_frameID  ;
		    channelBrElement.tacID     = hit_tacID    ;
		    channelBrElement.tCoarse   = hit_tCoarse  ;
		    channelBrElement.eCoarse   = hit_eCoarse  ;
		    channelBrElement.tFine     = hit_tFine    ;
		    channelBrElement.eFine     = hit_eFine    ;
				channelBrElement.timeBegin = hit_timeBegin;
				channelBrElement.timeEnd   = hit_timeEnd  ;
				channelBrElement.qdc_cal   = hit_qdc_cal  ;

				vBranchTemp.push_back( channelBrElement );
			}

			//fTreeCoin->Fill();
			vTree.push_back( vBranchTemp );
		}
	}

	//fTreeCoin->Print();
	std::cout << "ok3. vTree.size() " << vTree.size() << std::endl;

	std::cout << "[TOF_CoincidenceEvents::getCoincidenceEvents] Completed" << std::endl;

	//return fTreeCoin;
	return vTree;
}


TTree* TOF_CoincidenceEvents::getCoincidenceEventsTree()
{
	if( !fTree ) {
		std::cout << "[ERR] TOF_QdcCalibMethod::getCoincidenceEvents(). fTree is NULL. Exit." << std::endl;
		return nullptr;
	}

	if( fTree->GetEntries() == 0 ) {
		std::cout << "[ERR] TOF_QdcCalibMethod::getCoincidenceEvents(). fTree's entries = 0. Exit." << std::endl;
		return nullptr;
	}

	vBranch.clear();


	/// create fTreeCoin and do Branch()
	fTreeCoin = new TTree( "tCoin", "tCoin" );
	fTreeCoin->SetDirectory(0);
	auto br = fTreeCoin->Branch( "coinEvt", &vBranch, 64000, 0 );

	fTree->GetEntry(0);
	long long frameID0= fTree->getFrameID(); 
	long long lastTime= CLOCKS_IN_A_FRAME * frameID0 + fTree->getTCoarse(); 	
	long long currTime= -1; // 64-bit signed int

	TOF_CoincidenceChannelInfo channelInfo{};
	TOF_CoincidenceChannelInfo channelBrElement{};
	std::map< uint32_t, TOF_CoincidenceChannelInfo> vChannelData;
	std::map< uint32_t, bool > vHitChannel;

	const int chanSize = fActiveChannelList.size();
	
	for( int i=0; i<fTree->GetEntries(); i++ )
	{
		fTree->GetEntry(i);

		auto frameID      = fTree->getFrameID();
		auto tCoarse      = fTree->getTCoarse();
		auto absChannelID = fTree->getChannelID(); // absolute channel ID

		channelInfo.frameID   = frameID;
		channelInfo.channelID = absChannelID;
		channelInfo.tacID   = fTree->getTacID();
		channelInfo.tCoarse = fTree->getTCoarse();
		channelInfo.eCoarse = fTree->getECoarse();
		channelInfo.tFine   = fTree->getTFine()  ;
		channelInfo.eFine   = fTree->getEFine()  ;

		long long time_trigCh;
		if( absChannelID == fTrigChannelID ) time_trigCh = CLOCKS_IN_A_FRAME * frameID + tCoarse;

		/// current time in clock
		currTime = CLOCKS_IN_A_FRAME * frameID + tCoarse;
		
			
		/// coincidence events within 'coincidenceT' clocks
		if( fabs(currTime - lastTime) > fCoinTimeWindow) 
		{
			vChannelData.clear();
			vHitChannel.clear();
		}

		for( auto activeChanID: fActiveChannelList )
		{
		  if( absChannelID == activeChanID )
		  {
				vHitChannel [absChannelID] = true;
		    vChannelData[absChannelID] = channelInfo;
			}
		}

		if( vChannelData.size() == 1 ) lastTime = currTime;
		
		bool good=1;
		for( auto activeChanID: fActiveChannelList )
		{
			good &= vHitChannel[ activeChanID ];
		}

		if( i==0 )
		std::cout << Form("[%03d] good: %d, currTime: %10lld, frameID= %10lld, tCoarse= %10hu, tFine= %10hu, channelID= %3u", i, good, currTime, frameID, channelInfo.tCoarse, channelInfo.tFine, channelInfo.channelID ) << std::endl;

		if( vChannelData.size() == fActiveChannelList.size() && good )
		{
			vBranch.clear();

			//for( const auto [chan, chanData]: vChannelData )
				//std::cout << Form("coincidence channels: %03d, idx = %03d", chanData.channelID, chan ) << std::endl;

		  for( auto activeChanID: fActiveChannelList )
			{
				auto hit_channelID = activeChanID;
		    auto hit_frameID   = vChannelData[activeChanID].frameID;
		    auto hit_tacID     = vChannelData[activeChanID].tacID  ;
		    auto hit_tCoarse   = vChannelData[activeChanID].tCoarse;
		    auto hit_eCoarse   = vChannelData[activeChanID].eCoarse;
		    auto hit_tFine     = vChannelData[activeChanID].tFine  ;
		    auto hit_eFine     = vChannelData[activeChanID].eFine  ;

				//std::cout << Form("[%03d] [coincidence] channelID: %03d, frameID= %10lld, tacID= %d, tCoarse= %10hu, tFine= %10hu, eCoarse= %10hu, eFine= %10hu,", i, hit_channelID, hit_frameID, hit_tacID, hit_tCoarse, hit_tFine, hit_eCoarse, hit_eFine ) << std::endl;

				auto hit_timeBegin = TOF_TdcQdcCalibration::getInstance()->getCalibratedTime( TOF_Branch::fBranchT, hit_channelID, hit_tacID, hit_frameID, hit_tCoarse, hit_tFine );
				//auto hit_timeBegin = theCalib->getCalibratedTime( TOF_Branch::fBranchT, hit_channelID, hit_tacID, hit_frameID, hit_tCoarse, hit_tFine );
				
				//std::cout << Form("[%03d] [coincidence] channelID: %03d, t_begin: %8.2f", i, hit_channelID, hit_timeBegin ) << std::endl;

				//auto ecoarse = channelBrElement.eCoarse;
				auto ecoarse = hit_eCoarse;
	      if((hit_eCoarse - hit_tCoarse) < -256) ecoarse += 1024;
				auto hit_timeEnd = double((frameID*1024+ ecoarse));
				
				//std::cout << Form("[%03d] [coincidence] channelID: %03d, t_begin: %8.2f, t_end: %8.2f", i, hit_channelID, hit_timeBegin, hit_timeEnd ) << std::endl;

				double hit_qdc_cal = 0;
				if( fQdcCalibMethod == TOF_QdcCalibMethod::fGetEnergy )
					hit_qdc_cal = TOF_TdcQdcCalibration::getInstance()->getEnergy( hit_channelID, hit_tacID, hit_frameID, hit_eCoarse, hit_eFine, hit_timeBegin  );
					//hit_qdc_cal = theCalib->getEnergy( hit_channelID, hit_tacID, hit_frameID, hit_eCoarse, hit_eFine, hit_timeBegin  );
				else if( fQdcCalibMethod == TOF_QdcCalibMethod::fLinear ) 
					hit_qdc_cal = 0; ////////////////////////////////
				else if( fQdcCalibMethod == TOF_QdcCalibMethod::fTiming ) 
					hit_qdc_cal =	TOF_TdcQdcCalibration::getInstance()->getCalibratedQDC( hit_channelID, hit_tacID, hit_frameID, hit_eCoarse, hit_eFine, hit_tCoarse, hit_timeBegin  );
					//hit_qdc_cal =	theCalib->getCalibratedQDC( hit_channelID, hit_tacID, hit_frameID, hit_eCoarse, hit_eFine, hit_tCoarse, hit_timeBegin  );
				else hit_qdc_cal = 0;
				
				channelBrElement.channelID = hit_channelID;
		    channelBrElement.frameID   = hit_frameID  ;
		    channelBrElement.tacID     = hit_tacID    ;
		    channelBrElement.tCoarse   = hit_tCoarse  ;
		    channelBrElement.eCoarse   = hit_eCoarse  ;
		    channelBrElement.tFine     = hit_tFine    ;
		    channelBrElement.eFine     = hit_eFine    ;
				channelBrElement.timeBegin = hit_timeBegin;
				channelBrElement.timeEnd   = hit_timeEnd  ;
				channelBrElement.qdc_cal   = hit_qdc_cal  ;

				vBranch.push_back( channelBrElement );
			}

			fTreeCoin->Fill();
		}
	}

	std::cout << "[TOF_CoincidenceEvents::getCoincidenceEvents] Completed" << std::endl;

	return fTreeCoin;
}

void TOF_CoincidenceEvents::reset()
{
  if(fTree) 
	{
    // fTree는 CloneTree()로 생성된 객체이므로 삭제해야 함
    fTree->ResetBranchAddresses(); // 이미 Clone된 객체에선 필요 없을 수 있음
    delete fTree;
    fTree=nullptr;
		//std::cout << "[TOF_CoincidenceEvents::reset()] fTree is reset" << std::endl;
  }
  
  vTree.clear();

	delete fHisto_dT;
	delete fHisto_NbOfEvt;
	delete fHisto_TvsQcal;
	fHisto_dT = nullptr;
  fHisto_NbOfEvt = nullptr;
  fHisto_TvsQcal = nullptr;		

	std::cout << "[TOF_CoincidenceEvents::reset()] Reset Completed" << std::endl;
};


TOF_CoincidenceEvents::~TOF_CoincidenceEvents()
{
	std::cout << "[TOF_CoincidenceEvents] destructor called" << std::endl;
	reset();
	if( this==theCoin ) theCoin=nullptr;
}
	
void TOF_CoincidenceEvents::generateHistoForQA(const char* pdfName)
{
	if( !fTreeCoin ){
		std::cerr << "[TOF_CoincidenceEvents] fTreeCoin does NOT exist." << std::endl;
		return;
	}

	if( fTreeCoin->GetEntries()==0 )
	{
		std::cerr << "[TOF_CoincidenceEvents] fTreeCoin has 0 entries." << std::endl;
		return;
	}

	if( fActiveChannelList.size()!=4 ) {
		std::cerr << "The number of active channels is not 4. Cannot proceed..Sorry, we are working on developing.." << std::endl;
		return;
	}

	uint32_t channel0 = fActiveChannelList.at(0);

	fHisto_dT = new TH1D("h_dT", "Time Diff between Paddles;Asym. time ratio (ns);", 300, -15, 15 );
	fHisto_NbOfEvt = new TH1D("hNevt", ";Channel;Number of Coincidence events", 128*8, 0, 128*8);
	fHisto_TvsQcal = new TH2D("hTvsQcal", ";Time diff in clock;", 300, -3, 3, 100, -0.6, 0.6);
		
	double ttimeBegin[4];
	double qqdc[4];
	double qqdc_cal[4];


	std::vector<TOF_CoincidenceChannelInfo>* vBranchA = nullptr;
	fTreeCoin->SetBranchAddress( "coinEvt", &vBranchA );

	for( int i=0; i<fTreeCoin->GetEntries(); i++ )
	{
		fTreeCoin->GetEntry(i);

	  std::sort( vBranchA->begin(), vBranchA->end(), [](const TOF_CoincidenceChannelInfo& a, const TOF_CoincidenceChannelInfo& b) { return a.channelID < b.channelID; });

		//std::cout << "Number of elements in a branch: " << vBranchA.size() << std::endl;
	  for( auto ele: *vBranchA ) 
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

			//std::cout << Form("[%04d] frameID: %lld, channel: %u, channel idx: %d, timeBegin: %4.3f", i, frameID, channelID, chanIdx, timeBegin) << std::endl;
			
			ttimeBegin[chanIdx] = timeBegin;
			qqdc[chanIdx] = eFine;
			qqdc_cal[chanIdx] = qdc_cal;
			//cout << Form( "channelID: %03d, channel idx: %d, (A, B, C)= (%d, %d, %d)", channelID, chanIdx, chanA, chanB, chanC) << endl;
		
			fHisto_NbOfEvt->Fill( channelID );
		}

		//cout << Form("channels: %03d, %03d, %03d, %03d", vBranchA->at(0).channelID,  vBranchA->at(1).channelID,  vBranchA->at(2).channelID,  vBranchA->at(3).channelID) << endl;

		double tdiff0 = ttimeBegin[0] - ttimeBegin[1];
		double tdiff1 = ttimeBegin[2] - ttimeBegin[3];
		double qCratio0 = (qqdc_cal[0] - qqdc_cal[1])/(qqdc_cal[0] + qqdc_cal[1]);
		double qCratio1 = (qqdc_cal[2] - qqdc_cal[3])/(qqdc_cal[2] + qqdc_cal[3]);

		//std::cout << Form("tdiff0: %4.3f, tdiff1: %4.2f", tdiff0, tdiff1) << std::endl;

		double dT = ( (ttimeBegin[0]-ttimeBegin[2]) + (ttimeBegin[1]-ttimeBegin[3]) )/2.0* fTdcClkNs; // ns
		//std::cout << Form("time: %4.4f", dT) << std::endl;
		fHisto_dT->Fill( dT );

		fHisto_TvsQcal->Fill( tdiff0, qCratio0 );
		fHisto_TvsQcal->Fill( tdiff1, qCratio1 );

		//if( i>100) break;
	}

  auto theAttrib   = TOF_Attributes::getInstance();



	gStyle->SetOptStat(111111);
	gStyle->SetOptFit(1111);
	TCanvas* canv00 = new TCanvas("canv00", "canv00");
	fHisto_dT->Draw();
	auto theFit = TOF_Fitting::getInstance();
	theFit->fitGauss( fHisto_dT, 2.5 );
	auto fit = theFit->getFitFunction();
	theAttrib->drawTextNDC( 0.05, 12, kRed, 0.15, 0.85, Form("t_resol= %.3f ns", fit->GetParameter(2)) );
	//canv00->Print("coincidence_dT.png");
  canv00->Print( Form("%s(", pdfName) );
	
	TCanvas* canv01 = new TCanvas("canv01", "canv01");
	fHisto_TvsQcal->Draw("colz");
	gPad->SetGrid();
	auto correlR = fHisto_TvsQcal->GetCorrelationFactor();
	theAttrib->drawTextNDC( 0.05, 12, kBlack, 0.15, 0.85, Form("correl= %.2f", correlR) );
	//canv01->Print("coincidence_TvsQcal.png");
  canv01->Print( pdfName );

	TCanvas* canv02 = new TCanvas("canv02", "canv02");
	fHisto_NbOfEvt->Draw();
	gPad->SetGrid();
	//canv02->Print("coincidence_CHvsNbOfEvt.png");
  canv02->Print( Form("%s)", pdfName) );

	canv00->Close();
	canv01->Close();
	canv02->Close();

	delete canv00;
	delete canv01;
	delete canv02;

	return;
}
