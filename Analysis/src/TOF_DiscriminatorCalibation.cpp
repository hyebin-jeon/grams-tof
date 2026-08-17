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
  std::ifstream fin;
	fin.clear();
	fin.open( fname );
	if( ! fin.is_open() ) {
		std::cout<< Form( "[ERR] DISCRIMINATOR calibration file does not exist.Exit(): %s", fname ) << std::endl;
		return TOF_ERR;
	}
	else { std::cout << "[INFO] TOF_DiscriminatorCalibration::readCalib --> reading " << fname << std::endl; }

	initializeParams();

  std::string word, sLine;
  std::stringstream ssLine;
	const int line0 = 1;
  int wordN{0}, lineN{0};
	//int portID, slaveID, chipID, channelID, baseline_T, baseline_E;
  unsigned short portID, slaveID, chipID, channelID, baseline_T, baseline_E;
	double zero_T1, zero_T2, zero_E;
	double noise_T1, noise_T2, noise_E;

	/// read line by line
  while( std::getline(fin, sLine) )
  {
    //ssLine.clear();
    //ssLine << sLine;
		ssLine.str(sLine);
    ssLine.clear();
		  
		if( lineN< line0 ) {lineN++; continue;}

		/// break a line to words
		/// the scan param table should use '\t' to separate variables
    wordN=0;
    while( std::getline(ssLine, word, '\t') ) 
    {   
			if     ( wordN==0 ) portID    = std::atoi( word.c_str() );
			else if( wordN==1 ) slaveID   = std::atoi( word.c_str() );
			else if( wordN==2 ) chipID    = std::atoi( word.c_str() );
			else if( wordN==3 ) channelID = std::atoi( word.c_str() );
			else if( wordN==4 ) baseline_T= std::atoi( word.c_str() );
			else if( wordN==5 ) baseline_E= std::atoi( word.c_str() );
			else if( wordN==6 ) zero_T1   = std::atof( word.c_str() );
			else if( wordN==7 ) zero_T2   = std::atof( word.c_str() );
			else if( wordN==8 ) zero_E    = std::atof( word.c_str() );
			else if( wordN==9 ) noise_T1  = std::atof( word.c_str() );
			else if( wordN==10) noise_T2  = std::atof( word.c_str() );
			else if( wordN==11) noise_E   = std::atof( word.c_str() );
			else std::cout << "[Warning] Too Many Scan Parameter values.." << std::endl;

			wordN++;
		}

		fBaseline[chipID][channelID][(int)fbranchT] = baseline_T;
    fBaseline[chipID][channelID][(int)fbranchE] = baseline_E;

    fZero[chipID][channelID][(int)fDiscrT1] = zero_T1;
    fZero[chipID][channelID][(int)fDiscrT2] = zero_T2;
    fZero[chipID][channelID][(int)fDiscrE]  = zero_E ;

    fNoise[chipID][channelID][(int)fDiscrT1] = noise_T1;
    fNoise[chipID][channelID][(int)fDiscrT2] = noise_T2;
    fNoise[chipID][channelID][(int)fDiscrE]  = noise_E ;

		//if( lineN<130 )
		//std::cout << Form("[%3d] chip: %02d, channel: %02d, BL_T: %2d, BL_E: %2d, zero_t1: %2.1f, zero_t2: %2.1f, zero_e: %2.1f, noise_t1: %2.1f, noise_t2: %2.1f, noise_e: %2.1f", lineN, chipID, channelID, baseline_T, baseline_E, zero_T1, zero_T2, zero_E, noise_T1, noise_T2, noise_E) << std::endl;
		lineN++;
	}

	//if( lineN>=130 ) return TOF_ERR;

	fin.close();
  std::cout << Form( "DISCRIMINATOR Calibration Data (%d lines) Loaded.", lineN ) << std::endl;

	return TOF_GOOD;
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

	std::cout << Form("chip: %02d, channel: %02d, BL_T: %2d, BL_E: %2d, zero_t1: %2.1f, zero_t2: %2.1f, zero_e: %2.1f, noise_t1: %2.1f, noise_t2: %2.1f, noise_e: %2.1f", 
			               chipID, channelID, 
										 fBaseline[chipID][channelID][fbranchT], fBaseline[chipID][channelID][fbranchE], 
										 fZero[chipID][channelID][fDiscrT1], fZero[chipID][channelID][fDiscrT2], fZero[chipID][channelID][fDiscrE],
										 fNoise[chipID][channelID][fDiscrT1], fNoise[chipID][channelID][fDiscrT2], fNoise[chipID][channelID][fDiscrE] 
									 ) << std::endl;

	return;
}

int TOF_DiscriminatorCalibration::getBaseline(uint8_t chipID, uint32_t channelID, TOF_Branch br )
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

	//std::cout << "TOF_DiscriminatorCalibration::getDiscrParams --> disc = " << (int) disc << std::endl;

	int brIdx(-99);
	if( disc== TOF_Discriminator::fDiscT1 || disc == TOF_Discriminator::fDiscT2 ) brIdx = (int) TOF_Branch::fBranchT;
	if( disc == TOF_Discriminator::fDiscE ) brIdx = (int) TOF_Branch::fBranchE;
	//std::cout << "TOF_DiscriminatorCalibration::getDiscrParams --> br = " << (int) brIdx << std::endl;

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

