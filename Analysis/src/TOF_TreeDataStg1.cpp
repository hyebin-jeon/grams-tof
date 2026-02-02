
#include "TOF_TreeDataStg1.h"

ClassImp( TOF_TreeDataStg1 );

int TOF_TreeDataStg1::isFileStg( const char* fpath )
{
	if( !((TString) fpath).EndsWith( Form(".stg%d.root", getStgNb()) ) ) {
		std::cout << Form("[WARN] Input Stg%d File does NOT end with \'.stg1.root\'", getStgNb()) << std::endl;
		return TOF_WARNING;
	}

	return TOF_GOOD;
}

int TOF_TreeDataStg1::setInputPath( const char* fpath )
{
	if ( ! std::filesystem::exists(fpath) ) {
		std::cerr<< Form( "[ERR] Input file does NOT exist.: %s", fpath ) << std::endl;
		return TOF_ERR_OUT_OF_RANGE;
	}
	else {
	  std::cout << Form("[Info] Stg%d Input File: ", getStgNb()) << fpath << std::endl;
	}

	isFileStg( fpath );

	fFileExist = true;
	fFilePath = fpath;
	fFileName = std::filesystem::path(fpath).filename().string();

	fTFile = new TFile( fpath, "read" );
	fTTree = (TTree*) fTFile->Get( "data" );
	if( !fTTree ) {
		std::cerr<< Form( "[ERR] TTree does NOT exist in %s:", fpath ) << std::endl;
		return TOF_ERR_OUT_OF_RANGE;
	}

	return TOF_GOOD;
}

void TOF_TreeDataStg1::setOutputPath( const char* fpath, const char* opt )
{
	if( std::string(opt) == "recreate" || std::string(opt) == "create" ) 
	{
	  if( fTFile ) fTFile = nullptr;
	  fTFile = new TFile( fpath, opt );
	  fFileExist = true;
	}

	fFilePath = fpath;
	fFileName = std::filesystem::path(fpath).filename().string();
	return;
}

void TOF_TreeDataStg1::setBranchStatus( const char* bname, bool status )
{
	if(!fTTree ) {
		std::cout<< Form("[WARN] Generate Stg%d TTree",getStgNb()) << std::endl;
		fTTree = new TTree("data", "data");
	}

	fTTree->SetBranchStatus( bname, status );
}

void TOF_TreeDataStg1::setBranchAddress()
{
	if(!fTTree ) {
		std::cout<< Form("[WARN] Generate Stg%d TTree", getStgNb()) << std::endl;
		fTTree = new TTree("data", "data");
	}

	int status;

  status = fTTree->SetBranchAddress("step1"    , &step1    );
	if (status < 0) std::cerr << "[ERR] Failed to set address for step1 (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("step2"    , &step2    );
	if (status < 0) std::cerr << "[ERR] Failed to set address for step2 (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("frameID"  , &frameID  );
	if (status < 0) std::cerr << "[ERR] Failed to set address for frameID (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("channelID", &channelID);
	if (status < 0) std::cerr << "[ERR] Failed to set address for channelID (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("tacID"    , &tacID    );
	if (status < 0) std::cerr << "[ERR] Failed to set address for tacID (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("tcoarse"  , &tCoarse  );
	if (status < 0) std::cerr << "[ERR] Failed to set address for tcoarse (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("ecoarse"  , &eCoarse  );
	if (status < 0) std::cerr << "[ERR] Failed to set address for ecoarse (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("tfine"    , &tFine    );
	if (status < 0) std::cerr << "[ERR] Failed to set address for tfine (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("efine"    , &eFine    );
	if (status < 0) std::cerr << "[ERR] Failed to set address for efine (Status: " << status << ")" << std::endl;
  
	return;
}

void TOF_TreeDataStg1::makeBranches()
{
	if(!fTTree ) {
		std::cout<< Form("[WARN] Generate Stg%d TTree", getStgNb()) << std::endl;
		fTTree = new TTree("data", "data");
	}

	fTTree->Branch("step1"    , &step1     );
	fTTree->Branch("step2"    , &step2     );
	fTTree->Branch("frameID"  , &frameID   );
	fTTree->Branch("channelID", &channelID );
	fTTree->Branch("tacID"    , &tacID     );
	fTTree->Branch("tcoarse"  , &tCoarse   );
	fTTree->Branch("ecoarse"  , &eCoarse   );
	fTTree->Branch("tfine"    , &tFine     );
	fTTree->Branch("efine"    , &eFine     );

	return;
}

		
std::string TOF_TreeDataStg1::getFilePath()
{
	if( ! fFileExist ) return "no_file_found";
	return fFilePath;
}
std::string TOF_TreeDataStg1::getFileName()
{
	if( ! fFileExist ) return "no_file_found";
	return fFileName;
}

TTimeStamp TOF_TreeDataStg1::getTimestampCPU()
{

	TTimeStamp cpu_ts0;
	try {
	  int year  = std::stoi(fFileName.substr( 4, 4));
    int month = std::stoi(fFileName.substr( 9, 2));
    int day   = std::stoi(fFileName.substr(12, 2));
    int hour  = std::stoi(fFileName.substr(15, 2));
    int min   = std::stoi(fFileName.substr(18, 2));
    int sec   = std::stoi(fFileName.substr(21, 2));
	  int ms    = std::stoi(fFileName.substr(24, 3));

    cpu_ts0 = TTimeStamp(year, month, day, hour, min, sec, ms*1000000);
	} 
	catch( const std::exception& e ) {
		std::cerr << "[ERR] TTimeStamp TOF_TreeDataStg1::getTimestampCPU() | Failed to parse Date&Time from input file name: " << e.what() << std::endl;
		cpu_ts0.Set();
	}

	return cpu_ts0;
}

