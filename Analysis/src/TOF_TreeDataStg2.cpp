
#include "TOF_TreeDataStg2.h"

ClassImp( TOF_TreeDataStg2 );

void TOF_TreeDataStg2::setBranchAddress()
{
	if(!fTTree ) {
		std::cout<< Form("[WARN] Generate Stg%d TTree", getStgNb()) << std::endl;
		fTTree = new TTree("data", "data");
	}

	int status;

  status = fTTree->SetBranchAddress("ts_cpu"    , &ts_cpu    );
	if (status < 0) std::cerr << "[ERR] Failed to set address for ts_cpu (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("ts_pps"    , &ts_pps    );
	if (status < 0) std::cerr << "[ERR] Failed to set address for ts_pps (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("step1"    , &step1    );
	if (status < 0) std::cerr << "[ERR] Failed to set address for step1 (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("step2"    , &step2    );
	if (status < 0) std::cerr << "[ERR] Failed to set address for step2 (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("frameID"  , &frameID  );
	if (status < 0) std::cerr << "[ERR] Failed to set address for frameID (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("channelID", &channelID);
	if (status < 0) std::cerr << "[ERR] Failed to set address for channelID (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("connID_febD", &connID_febD);
	if (status < 0) std::cerr << "[ERR] Failed to set address for connID_febD (Status: " << status << ")" << std::endl;
  status = fTTree->SetBranchAddress("connID_febS", &connID_febS);
	if (status < 0) std::cerr << "[ERR] Failed to set address for connID_febS (Status: " << status << ")" << std::endl;
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


void TOF_TreeDataStg2::makeBranches()
{
	if(!fTTree ) {
		std::cout << Form("[WARN] Generate Stg%d TTree", getStgNb()) << std::endl;
		fTTree = new TTree("data", "data");
	}

	fTTree->Branch("ts_cpu"     , &ts_cpu, 32000, 0);
	fTTree->Branch("ts_pps"     , &ts_pps, 32000, 0);
	fTTree->Branch("step1"      , &step1       );
	fTTree->Branch("step2"      , &step2       );
	fTTree->Branch("frameID"    , &frameID     );
	fTTree->Branch("channelID"  , &channelID   );
	fTTree->Branch("connID_febD", &connID_febD );
	fTTree->Branch("connID_febS", &connID_febS );
	fTTree->Branch("tacID"      , &tacID       );
	fTTree->Branch("tcoarse"    , &tCoarse     );
	fTTree->Branch("ecoarse"    , &eCoarse     );
	fTTree->Branch("tfine"      , &tFine       );
	fTTree->Branch("efine"      , &eFine       );

	return;
}

