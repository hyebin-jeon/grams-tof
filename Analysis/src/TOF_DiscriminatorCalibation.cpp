#include "TOF_DiscriminatorCalibation.h"

ClassImp( TOF_DiscriminatorCalibration );

//TOF_DiscriminatorCalibration::TOF_DiscriminatorCalibration()
void TOF_DiscriminatorCalibration::initializeParams()
{
	int chipN    = NUMBER_OF_ASICS;
	int channelN = NUMBER_OF_CHANNELS_OF_ASIC;
	int branchN    = 2; // T or E 

	/// initialization
  for ( int chipID = 0; chipID<chipN; chipID++ ) 
	{
    for ( int chanID = 0; chanID<channelN; chanID++ ) 
		{
			for( int br=0; br<branchN; br++ ) fBaseline[chipID][chanID][br] = -99999;

			for( int disc=0; disc<fNbOfDiscrimators; disc++ )
			{
				fZero [chipID][chanID][disc] = -99999;
				fNoise[chipID][chanID][disc] = -99999;
			}

			//B_T [chipID][chanID][brID] = -99999;
	    //B_E [chipID][chanID][brID] = -99999;
	    //Z_t1[chipID][chanID][brID] = -99999;
	    //Z_t2[chipID][chanID][brID] = -99999;
	    //Z_e [chipID][chanID][brID] = -99999;
	    //N_t1[chipID][chanID][brID] = -99999;
	    //N_t2[chipID][chanID][brID] = -99999;
	    //N_e [chipID][chanID][brID] = -99999;
    }
  }

	return;
}

//double TOF_DiscriminatorCalibration::getBaseline( uint8_t chipID, uint32_t channelID, TOF_Branch br ) { 
//	if( br == TOF_Branch::fBranchT ) return getT0_T(chipID, channelID, tacID);
//	if( br == TOF_Branch::fBranchE ) return getT0_E(chipID, channelID, tacID);
//
//	return -99;
//};

int TOF_DiscriminatorCalibration::readCalib( const char *fname )
{
  std::ifstream finQ( fname );
	if( ! finQ.is_open() ) {
		std::cout<< Form( "[ERR] DISCRIMINATOR calibration file does not exist.Exit(): %s", fname ) << std::endl;
		return TOF_ERR;
	}
  unsigned short portID, slaveID, chipID, channelID;
  double bT, bE, zT1, zT2, zE, nT1, nT2, nE;
  TString head;
  char buf[256];
  unsigned short ndata = 0;
  do {
    finQ >> head;
    if ( head.Contains( '#' ) ) {
      finQ.getline( buf, 256 );
      continue;
    }
    if ( finQ.eof() ) break;
    finQ >> slaveID >> chipID >> channelID >> bT >> bE >> zT1 >> zT2 >> zE >> nT1 >> nT2 >> nE;
    //    std::cout << Form( "%u %u", chipID, channelID ) << std::endl;
    fBaseline[chipID][channelID][fbranchT] = bT;
    fBaseline[chipID][channelID][fbranchE] = bE;

    fZero[chipID][channelID][fDiscrT1] = zT1;
    fZero[chipID][channelID][fDiscrT2] = zT2;
    fZero[chipID][channelID][fDiscrE]  = zE ;

    fNoise[chipID][channelID][fDiscrT1] = nT1;
    fNoise[chipID][channelID][fDiscrT2] = nT2;
    fNoise[chipID][channelID][fDiscrE]  = nE ;

		//std::cout << Form("chip: %02d, channel: %02d, BL_T: %2.1f, BL_E: %2.1f, zero_t1: %2.1f, zero_t2: %2.1f, zero_e: %2.1f, noise_t1: %2.1f, noise_t2: %2.1f, noise_e: %2.1f", chipID, channelID, bT, bE, zT1, zT2, zE, nT1, nT2, nE) << std::endl;
		ndata++;
  } while( 1 );
  std::cout << Form( "DISCRIMINATOR Calibration Data (%d lines) Loaded.", ndata ) << std::endl;

	return 1;
}

int TOF_DiscriminatorCalibration::readCalibFromDir( const char* dirPath )
{
	const char* thr_calib = Form( "%s/disc_calibration.tsv", dirPath );
	auto ok = readCalib( thr_calib );

	return ok;
}

void TOF_DiscriminatorCalibration::printCalibTable( uint32_t absChannelID )
{
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );

	std::cout << Form("chip: %02d, channel: %02d, BL_T: %2.1f, BL_E: %2.1f, zero_t1: %2.1f, zero_t2: %2.1f, zero_e: %2.1f, noise_t1: %2.1f, noise_t2: %2.1f, noise_e: %2.1f", 
			               chipID, channelID, 
										 fBaseline[chipID][channelID][fbranchT], fBaseline[chipID][channelID][fbranchE], 
										 fZero[chipID][channelID][fDiscrT1], fZero[chipID][channelID][fDiscrT2], fZero[chipID][channelID][fDiscrE],
										 fNoise[chipID][channelID][fDiscrT1], fNoise[chipID][channelID][fDiscrT2], fNoise[chipID][channelID][fDiscrE] 
									 ) << std::endl;

	return;
}

double TOF_DiscriminatorCalibration::getBaseline(uint8_t chipID, uint32_t channelID, TOF_Branch br )
{
	int brIdx = (int) br; 
	return fBaseline[chipID][channelID][brIdx];
}
double TOF_DiscriminatorCalibration::getZero(uint8_t chipID, uint32_t channelID, TOF_Discriminator disc )
{
	int dIdx = (int) disc; 
	return fZero[chipID][channelID][dIdx];
}
double TOF_DiscriminatorCalibration::getNoise(uint8_t chipID, uint32_t channelID, TOF_Discriminator disc )
{
	int dIdx = (int) disc; 
	return fNoise[chipID][channelID][dIdx];
}
		
std::vector<double> TOF_DiscriminatorCalibration::getDiscrParams( uint32_t absChannelID, TOF_Discriminator disc )
{
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );

	int brIdx(-99);
	if( disc== TOF_Discriminator::fDiscT1 || disc == TOF_Discriminator::fDiscT2 ) brIdx = (int) TOF_Branch::fBranchT;
	if( disc == TOF_Discriminator::fDiscE ) brIdx = (int) TOF_Branch::fBranchE;

	int dIdx = (int) disc;

	auto baseline = fBaseline[chipID][channelID][brIdx];
	auto zero     = fZero[chipID][channelID][dIdx];
	auto noise    = fNoise[chipID][channelID][dIdx];

	std::vector<double> rval;
	rval.clear();
	rval.push_back( baseline );
	rval.push_back( zero     );
	rval.push_back( noise    );

	return rval;
}

