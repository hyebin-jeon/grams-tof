#include "TOF_ConvertStg1toStg2.h"
#include "GRAMS_TOF_Config.h"

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
	if( !fStg1 ) setClassStg1();
	return fStg1->setInputPath( fpath );
};


int TOF_ConvertStg1toStg2::addBranches()
{

	if( !fStg1->getTTree() ) {
		std::cerr << "[ERR] TOF_ConvertStg1toStg2::addTimestampBranches() | fStg1->getTTree() is NULL." << std::endl;
		return TOF_ERR;
	}

	if( fStg1->getEntries() == 0 ) {
		std::cout << "[WARN] TOF_ConvertStg1toStg2::addTimestampBranches() | fStg1 entries = 0." << std::endl;
		return TOF_ERR;
	}

	auto theChanConv = TOF_ChannelConversion::getInstance();
	auto theAsicList = TOF_ActiveAsicList::getInstance();
	auto thePaddle   = TOF_PaddleChannelMap::getInstance();

	fStg1->setBranchAddress(); // duplicate
	fStg1->getEntry(0);
	long long frameID0= fStg1->getFrameID(); 
	long long initialT= CLOCKS_IN_A_FRAME * frameID0 + fStg1->getTCoarse(); 	
	TTimeStamp ts_cpu0 = fStg1->getTimestampCPU();
	TTimeStamp ts_cpu = ts_cpu0;
	//TTimeStamp ts_pps = ts_cpu0;

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

		/// apply the calibration 
		double tdc_cal(0);
		double qdc_cal(0);
		if( fCalib ) {
		  tdc_cal = fCalib->getCalibratedTime( TOF_Branch::fBranchT, channelID, tacID, frameID, tCoarse, tFine ); // t_begin
		  qdc_cal = fCalib->getCalibratedQDC( channelID, tacID, frameID, eCoarse, eFine, tCoarse, tdc_cal );
		} else {
			if(i == 0) std::cout << "[WARN] Calibration parameters are not loaded. Writing zeros to time/charge." << std::endl;
		}

		/// t_end
		//auto ecoarse = eCoarse;
		//if((eCoarse - tCoarse) < -256) ecoarse += 1024;
		//auto t_end = double((frameID*1024+ ecoarse));
	
		/// physical channel ID (connector IDs)
		auto connID_D  = theChanConv->getConnIdOnFebD( channelID );
		auto connID_S  = theChanConv->getConnIdOnFebS( channelID );

		/// paddle IDs
		auto paddleIdx = thePaddle->getPaddleIdx( connID_D, connID_S );
		//auto paddleID = thePaddle->getPaddleLocId( connID_D, connID_S );

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

		fStg2->setStep1        ( step1     );
    fStg2->setStep2        ( step2     );
    fStg2->setStepBgin     ( stepBegin );
    fStg2->setStepEnd      ( stepEnd   );
    fStg2->setFrameID      ( frameID   );
    fStg2->setChannelID    ( channelID );
		fStg2->setConnID_FebD  ( connID_D  );
		fStg2->setConnID_FebS  ( connID_S  );
		fStg2->setPaddleIdx    ( paddleIdx );
    fStg2->setTacID        ( tacID     );
    fStg2->setTCoarse      ( tCoarse   );
    fStg2->setECoarse      ( eCoarse   );
    fStg2->setTFine        ( tFine     );
    fStg2->setEFine        ( eFine     );
    fStg2->setCalibratedTdc( tdc_cal   );
    fStg2->setCalibratedQdc( qdc_cal   );
		fStg2->setTimeStampCPU ( &ts_cpu   );

		fStg2->fillTTree(); 
	}
  
	return TOF_GOOD;
}

int TOF_ConvertStg1toStg2::loadCalibration( const std::string kTdcPath, const std::string kQdcPath ) 
{
	if( !kTdcPath.empty() && !kQdcPath.empty() ) {
		fTdcPath = kTdcPath;
		fQdcPath = kQdcPath;
	}
	else if( kTdcPath.empty() && kQdcPath.empty() ) return loadCalibration("");
	else if( kTdcPath.empty() || kQdcPath.empty() ){
		fTdcPath = kTdcPath;
		fQdcPath = kQdcPath;
		
		std::string dir_config = GRAMS_TOF_Config::instance().getConfigDir();
		if( kTdcPath.empty() ) fTdcPath = dir_config + "/tdc_calibration.tsv";
	  if( kQdcPath.empty() ) fQdcPath = dir_config + "/qdc_calibration.tsv";
	}
			
	fCalib = TOF_TdcQdcCalibration::getInstance();
	return fCalib->readCalibrationFiles( fTdcPath, fQdcPath );
}

int TOF_ConvertStg1toStg2::loadCalibration( const std::string kDirPath ) 
{
  std::string calibDirPath;
  
	if( kDirPath.empty() ) 
    calibDirPath = GRAMS_TOF_Config::instance().getConfigDir();
	else
		calibDirPath = kDirPath;
	
	if( !std::filesystem::is_directory( calibDirPath ) ) {
		std::cout << "[ERR] TOF_ConvertStg1toStg2::convertStg1ToStg2_wCalibDir() - Provide a calibration DIRECTORY path" << std::endl;
		return TOF_ERR;
	}
			
	fCalib = TOF_TdcQdcCalibration::getInstance();
	return fCalib->readCalibrationFiles( calibDirPath );
}

void TOF_ConvertStg1toStg2::convertStg1ToStg2( const char* kPathStg1, const char* kPathStg2, const char* tdc_cal_tsv, const char* qdc_cal_tsv  )
{
	/// load calibration files
	if( !fCalib ) loadCalibration(tdc_cal_tsv, qdc_cal_tsv);

	if( !fStg1 ) setClassStg1();
	if( !fStg2 ) setClassStg2();

	if( fStg1->setInputPath( kPathStg1 ) != TOF_GOOD ) return;

	if( strcmp(kPathStg2, "")==0 ) {
		TString dir   = std::filesystem::current_path().string() + "/output"; // Stg2 path 
		TString name1 = fStg1->getFileName();
		size_t idx = name1.Index(".stg1.root");
		TString name2 = name1(0, idx); 
		kPathStg2 = Form( "%s/%s.stg2.root", dir.Data(), name2.Data() );
	}

	/// stg2 TTree
	fStg2->setOutputPath( kPathStg2, "recreate" );
	addBranches(); // <- this required calibration files
	fStg2->getTTree()->Write();
	if( fStg2->getTFile()->GetListOfKeys()->GetEntries()>1 ) fStg2->getTFile()->Purge();
	std::cout << "[INFO] Stg2 File Generated With Timestamp: " << fStg2->getFilePath() << std::endl;
	fStg2->closeTFile();
 
  delete fStg1; 
  fStg1 = nullptr;
    
  delete fStg2; 
  fStg2 = nullptr;

	return;
}

