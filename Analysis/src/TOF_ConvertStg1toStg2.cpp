
#include "TOF_ConvertStg1toStg2.h"

ClassImp( TOF_ConvertStg1toStg2 );

void TOF_ConvertStg1toStg2::setClassStg1()
{
	fStg1 = new TOF_TreeDataStg1();
	return;
}
void TOF_ConvertStg1toStg2::setClassStg2()
{
	fStg2 = new TOF_TreeDataStg2();
	return;
}
int TOF_ConvertStg1toStg2::setInputPathStg1( const char* fpath )
{
	return fStg1->setInputPath( fpath );
};


//int TOF_ConvertStg1toStg2::addTimestampBranches()
int TOF_ConvertStg1toStg2::addBranches_TSandConnID()
{

	if( !fStg1->getTTree() ) {
		std::cerr << "[ERR] TOF_ConvertStg1toStg2::addTimestampBranches() | fStg1->getTTree() is NULL." << std::endl;
		return TOF_ERR;
	}

	if( fStg1->getEntries() == 0 ) {
		std::cout << "[WARN] TOF_ConvertStg1toStg2::addTimestampBranches() | fStg1 entries = 0." << std::endl;
		return TOF_ERR;
	}

	fStg1->setBranchAddress(); // duplicate
	fStg1->getEntry(0);
	long long frameID0= fStg1->getFrameID(); 
	long long initialT= CLOCKS_IN_A_FRAME * frameID0 + fStg1->getTCoarse(); 	
	TTimeStamp ts_cpu0 = fStg1->getTimestampCPU();
	TTimeStamp ts_cpu = ts_cpu0;
	TTimeStamp ts_pps = ts_cpu0;

	fStg2->makeBranches();

	for( int i=0; i<fStg1->getEntries(); i++ )
	{
		fStg1->getEntry(i);

		auto step1     = fStg1->getStep1()    ;
    auto step2     = fStg1->getStep2()    ;
    auto stepBegin = fStg1->getStepBgin() ;
    auto stepEnd   = fStg1->getStepEnd()  ;
    auto frameID   = fStg1->getFrameID()  ;
    auto channelID = fStg1->getChannelID();
    auto tacID     = fStg1->getTacID()    ;
    auto tCoarse   = fStg1->getTCoarse()  ;
    auto eCoarse   = fStg1->getECoarse()  ;
    auto tFine     = fStg1->getTFine()    ;
    auto eFine     = fStg1->getEFine()    ;
		
		auto connID_D  = TOF_ChannelConversion::getInstance()->getConnIdOnFebD( channelID );
		auto connID_S  = TOF_ChannelConversion::getInstance()->getConnIdOnFebS( channelID );

		long long currT = CLOCKS_IN_A_FRAME * frameID + tCoarse;
		long long diffT = currT - initialT;
		long long timeSec = diffT * fTOF_TdcClkPer; 
		long long timeNsec= (diffT * fTOF_TdcClkPerNs) - (timeSec*1000000000LL);

		ts_cpu = ts_cpu0;
		long long cpuSec  = ts_cpu0.GetSec() + timeSec;
		long long cpuNsec = ts_cpu0.GetNanoSec() + timeNsec;

		/// when Nsec < 1 sec
		if (cpuNsec >= 1000000000LL) {
        cpuSec += cpuNsec / 1000000000LL;
        cpuNsec %= 1000000000LL;
    }

		ts_cpu.SetSec( cpuSec );
		ts_cpu.SetNanoSec( cpuNsec );

		ts_pps = ts_cpu; // temporary dummy

		fStg2->setStep1       ( step1     );
    fStg2->setStep2       ( step2     );
    fStg2->setStepBgin    ( stepBegin );
    fStg2->setStepEnd     ( stepEnd   );
    fStg2->setFrameID     ( frameID   );
    fStg2->setChannelID   ( channelID );
		fStg2->setConnID_FebD ( connID_D  );
		fStg2->setConnID_FebS ( connID_S  );
    fStg2->setTacID       ( tacID     );
    fStg2->setTCoarse     ( tCoarse   );
    fStg2->setECoarse     ( eCoarse   );
    fStg2->setTFine       ( tFine     );
    fStg2->setEFine       ( eFine     );
		fStg2->setTimeStampCPU( &ts_cpu    );
		fStg2->setTimeStampPPS( &ts_pps    );

		fStg2->fillTTree(); 
	}
  
	return TOF_GOOD;
}

//int TOF_ConvertStg1toStg2::addConnIdBranches()
//{
//
//	if( !fStg1->getTTree() ) {
//		std::cerr << "[ERR] TOF_ConvertStg1toStg2::addTimestampBranches() | fStg1->getTTree() is NULL." << std::endl;
//		return TOF_ERR;
//	}
//
//	if( fStg1->getEntries() == 0 ) {
//		std::cout << "[WARN] TOF_ConvertStg1toStg2::addTimestampBranches() | fStg1 entries = 0." << std::endl;
//		return TOF_ERR;
//	}
//
//	fStg1->setBranchStatus("channelID",1); // duplicate
//	fStg1->setBranchAddress(); // duplicate
//
//	//fStg2->makeBranches();
//
//	for( int i=0; i<fStg1->getEntries(); i++ )
//	{
//		fStg1->getEntry(i);
//
//    auto channelID = fStg1->getChannelID();
//		auto connID_D  = TOF_ChannelConversion::getInstance()->getConnIdOnFebD( channelID );
//		auto connID_S  = TOF_ChannelConversion::getInstance()->getConnIdOnFebS( channelID );
//
//		fStg2->setConnID_FebD( connID_D );
//		fStg2->setConnID_FebS( connID_S );
//
//		//fStg2->fillTTree();
//	}
//
//	return TOF_GOOD;
//}



void TOF_ConvertStg1toStg2::convertStg1ToStg2( const char* kPathStg1, const char* kPathStg2 )
{
	if( !fStg1 ) setClassStg1();
	if( !fStg2 ) setClassStg2();

	if( fStg1->setInputPath( kPathStg1 ) < 0 ) return;

	if( strcmp(kPathStg2, "")==0 ) {
		TString dir   = std::filesystem::current_path().string() + "/output"; // Stg2 path 
		TString name1 = fStg1->getFileName();
		size_t idx = name1.Index(".stg1.root");
		TString name2 = name1(0, idx); 
		kPathStg2 = Form( "%s/%s.stg2.root", dir.Data(), name2.Data() );
	}

	fStg2->setOutputPath( kPathStg2, "recreate" );
	//fStg2->makeBranches();
	//addTimestampBranches();
	addBranches_TSandConnID();
	fStg2->getTTree()->Write();
	std::cout << "[INFO] Stg2 File Generated With Timestamp: " << fStg2->getFilePath() << std::endl;
	fStg2->closeTFile();
 
  delete fStg1; 
  fStg1 = nullptr;
    
  delete fStg2; 
  fStg2 = nullptr;
}

