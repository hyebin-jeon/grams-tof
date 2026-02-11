
//#include <fstream>
#include "TOF_TdcQdcCalibration.h"

ClassImp( TOF_TdcQdcCalibration );

//TOF_TdcQdcCalibration::TOF_TdcQdcCalibration()
void TOF_TdcQdcCalibration::initializeParams()
{
	int chipN    = NUMBER_OF_ASICS;
	int channelN = NUMBER_OF_CHANNELS_OF_ASIC;
	int tacN     = fNbOfTac;
	int branchN    = 2; // T or E 

	/// initialization
  for ( int chipID = 0; chipID<chipN; chipID++ ) 
	{
    for ( int chanID = 0; chanID<channelN; chanID++ ) 
		{
      for ( int tacID = 0; tacID<tacN; tacID++ ) 
			{
				P0[chipID][chanID][tacID] = -99999;
	      P1[chipID][chanID][tacID] = -99999;
	      P2[chipID][chanID][tacID] = -99999;
	      P3[chipID][chanID][tacID] = -99999;
	      P4[chipID][chanID][tacID] = -99999;
	      P5[chipID][chanID][tacID] = -99999;
	      P6[chipID][chanID][tacID] = -99999;
	      P7[chipID][chanID][tacID] = -99999;
	      P8[chipID][chanID][tacID] = -99999;

	      for ( int brID = 0; brID<branchN; brID++ ) 
				{
	         T0[chipID][chanID][tacID][brID] = -99999;
	         A0[chipID][chanID][tacID][brID] = -99999;
	         A1[chipID][chanID][tacID][brID] = -99999;
	         A2[chipID][chanID][tacID][brID] = -99999;
	      }
      }
    }
  }

	return;
}

double TOF_TdcQdcCalibration::getT0( uint8_t chipID, uint32_t channelID, uint8_t tacID, TOF_Branch br ) { 
	if( br == TOF_Branch::fBranchT ) return getT0_T(chipID, channelID, tacID);
	if( br == TOF_Branch::fBranchE ) return getT0_E(chipID, channelID, tacID);

	return -99;
};
double TOF_TdcQdcCalibration::getA0( uint8_t chipID, uint32_t channelID, uint8_t tacID, TOF_Branch br ) { 
	if( br == TOF_Branch::fBranchT ) return getA0_T(chipID, channelID, tacID);
	if( br == TOF_Branch::fBranchE ) return getA0_E(chipID, channelID, tacID);

	return -99;
};
double TOF_TdcQdcCalibration::getA1( uint8_t chipID, uint32_t channelID, uint8_t tacID, TOF_Branch br ) { 
	if( br == TOF_Branch::fBranchT ) return getA1_T(chipID, channelID, tacID);
	if( br == TOF_Branch::fBranchE ) return getA1_E(chipID, channelID, tacID);

	return -99;
};
double TOF_TdcQdcCalibration::getA2( uint8_t chipID, uint32_t channelID, uint8_t tacID, TOF_Branch br ) { 
	if( br == TOF_Branch::fBranchT ) return getA2_T(chipID, channelID, tacID);
	if( br == TOF_Branch::fBranchE ) return getA2_E(chipID, channelID, tacID);

	return -99;
};

int TOF_TdcQdcCalibration::readTdcCalib( const char *fname )
{
  std::ifstream finT( fname );
	if( ! finT.is_open() ) {
		std::cout<< Form( "[ERR] TDC calibration file does not exist.Exit(): %s", fname ) << std::endl;
		return TOF_ERR;
	}
  unsigned short portID, slaveID, chipID, channelID, tacID;
  char branch;
  double t0, a0, a1, a2;
  TString head;
  char buf[256];
  unsigned short ndata = 0;
  do {
    finT >> head;
    if ( head.Contains( '#' ) ) {
      finT.getline( buf, 256 );
      continue;
    }
    if ( finT.eof() ) break;
    finT >> slaveID >> chipID >> channelID >> tacID >> branch >> t0 >> a0 >> a1 >> a2;
    //    std::cout << Form( "%u %u", chipID, channelID ) << std::endl;
    if ( branch == 'T' ) {
      T0[chipID][channelID][tacID][0] = t0;
      A0[chipID][channelID][tacID][0] = a0;
      A1[chipID][channelID][tacID][0] = a1;
      A2[chipID][channelID][tacID][0] = a2;
    }
    else {
      T0[chipID][channelID][tacID][1] = t0;
      A0[chipID][channelID][tacID][1] = a0;
      A1[chipID][channelID][tacID][1] = a1;
      A2[chipID][channelID][tacID][1] = a2;
    }
		//std::cout << "[TDC calib] T0: " << t0 << std::endl;
    ndata++;
  } while( 1 );
  std::cout << Form( "TDC Calibration Data (%d lines) Loaded.", ndata ) << std::endl;

	return 1;
}

int TOF_TdcQdcCalibration::readQdcCalib( const char *fname )
{
  std::ifstream finQ( fname );
	if( ! finQ.is_open() ) {
		std::cout<< Form( "[ERR] QDC calibration file does not exist.Exit(): %s", fname ) << std::endl;
		return TOF_ERR;
	}
  unsigned short portID, slaveID, chipID, channelID, tacID;
  double p0, p1, p2, p3, p4, p5, p6, p7, p8, p9;
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
    finQ >> slaveID >> chipID >> channelID >> tacID >> p0 >> p1 >> p2 >> p3 >> p4 >> p5 >> p6 >> p7 >> p8 >> p9;
    //    std::cout << Form( "%u %u", chipID, channelID ) << std::endl;
    P0[chipID][channelID][tacID] = p0;
    P1[chipID][channelID][tacID] = p1;
    P2[chipID][channelID][tacID] = p2;
    P3[chipID][channelID][tacID] = p3;
    P4[chipID][channelID][tacID] = p4;
    P5[chipID][channelID][tacID] = p5;
    P6[chipID][channelID][tacID] = p6;
    P7[chipID][channelID][tacID] = p7;
    P8[chipID][channelID][tacID] = p8;
    P9[chipID][channelID][tacID] = p9;
		//std::cout << "[QDC calib] P0: " << p0 << std::endl;
		ndata++;
  } while( 1 );
  std::cout << Form( "QDC Calibration Data (%d lines) Loaded.", ndata ) << std::endl;

	return 1;
}

int TOF_TdcQdcCalibration::readCalibrationFiles( const char* fTdcCalib, const char* fQdcCalib )
{
	auto tdc = readTdcCalib( fTdcCalib );
	auto qdc = readQdcCalib( fQdcCalib );
	if( tdc<0 || qdc<0 ) return TOF_ERR;
	return 1;
}

int TOF_TdcQdcCalibration::readCalibrationFiles( const char* dirPath )
{
	const char* tdc_calib = Form( "%s/tdc_calibration.tsv", dirPath );
	const char* qdc_calib = Form( "%s/qdc_calibration.tsv", dirPath );
	auto ok = readCalibrationFiles( tdc_calib, qdc_calib );

	return ok;
}

std::vector<double> TOF_TdcQdcCalibration::getTdcParams_T( uint32_t absChannelID, uint8_t tacID )
{
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );
	
	std::vector<double> rval;

	rval.push_back( getT0_T( chipID, channelID, tacID ) );
	rval.push_back( getA0_T( chipID, channelID, tacID ) );
	rval.push_back( getA1_T( chipID, channelID, tacID ) );
	rval.push_back( getA2_T( chipID, channelID, tacID ) );

	return rval;
}
std::vector<double> TOF_TdcQdcCalibration::getTdcParams_E( uint32_t absChannelID, uint8_t tacID )
{
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );
	
	std::vector<double> rval;

	rval.push_back( getT0_E( chipID, channelID, tacID ) );
	rval.push_back( getA0_E( chipID, channelID, tacID ) );
	rval.push_back( getA1_E( chipID, channelID, tacID ) );
	rval.push_back( getA2_E( chipID, channelID, tacID ) );

	return rval;
}
std::vector<double> TOF_TdcQdcCalibration::getTdcParams( uint32_t absChannelID, uint8_t tacID )
{
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );
	
	std::vector<double> rval;

	rval.push_back( getT0_T( chipID, channelID, tacID ) );
	rval.push_back( getA0_T( chipID, channelID, tacID ) );
	rval.push_back( getA1_T( chipID, channelID, tacID ) );
	rval.push_back( getA2_T( chipID, channelID, tacID ) );
	rval.push_back( getT0_E( chipID, channelID, tacID ) );
	rval.push_back( getA0_E( chipID, channelID, tacID ) );
	rval.push_back( getA1_E( chipID, channelID, tacID ) );
	rval.push_back( getA2_E( chipID, channelID, tacID ) );


	printf("T0: %1.3e, A0: %1.3e, A1: %1.3e, A2: %1.3e\n", rval.at(0), rval.at(1), rval.at(2), rval.at(3) );

	return rval;
}

std::vector<double> TOF_TdcQdcCalibration::getQdcParams( uint32_t absChannelID, uint8_t tacID )
{
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );

	std::vector<double> rval;

	rval.push_back( getP0( chipID, channelID, tacID ) );
	rval.push_back( getP1( chipID, channelID, tacID ) );
	rval.push_back( getP2( chipID, channelID, tacID ) );
	rval.push_back( getP3( chipID, channelID, tacID ) );
	rval.push_back( getP4( chipID, channelID, tacID ) );
	rval.push_back( getP5( chipID, channelID, tacID ) );
	rval.push_back( getP6( chipID, channelID, tacID ) );
	rval.push_back( getP7( chipID, channelID, tacID ) );
	rval.push_back( getP8( chipID, channelID, tacID ) );
	rval.push_back( getP9( chipID, channelID, tacID ) );

	return rval;
}

void TOF_TdcQdcCalibration::printQdcCalibTable( uint32_t absChannelID )
{
	//auto chipID    = getChipID   ( absChannelID );
	//auto channelID = getChannelID( absChannelID );
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );

	printf("==============================================\n");
	printf("Absolute channel ID = %03d\n", absChannelID);
	printf("-> chipID= %2d, channelID in the chip= %2d\n", chipID, channelID );
	printf("==============================================\n");
	printf("tacID\t");
	for( int i=0; i<fNbOfQdcParams; i++ ) printf("p%d\t",i);
	printf("\n");

	for( int tac=0; tac<fNbOfTac; tac++)
	{
		printf("%d\t", tac );
		auto params = getQdcParams( absChannelID, tac );
		for( int i=0; i<fNbOfQdcParams; i++ )
		{
			printf("%2.2e\t", params.at(i)); 
		}
		printf("\n");
	}

	return;
}

void TOF_TdcQdcCalibration::printTdcCalibTable( uint32_t absChannelID ) 
{
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );

	printf("==============================================\n");
	printf("Absolute channel ID = %03d\n", absChannelID);
	printf("-> chipID= %2d, channelID in the chip= %2d\n", chipID, channelID );
	printf("==============================================\n");
	printf("br\ttacID\t");

	/// table content titles
	for( int i=0; i<fNbOfTdcParams; i++ ) printf("p%d\t",i);
	printf("\n");


	/// T branch
	for( int tac=0; tac<fNbOfTac; tac++)
	{
		printf("T\t%d\t", tac );
		auto params = getTdcParams_T( absChannelID, tac );
		for( int i=0; i<fNbOfTdcParams; i++ )
		{
			printf("%2.2e\t", params.at(i)); 
		}
		printf("\n");
	}

	/// E branch
	for( int tac=0; tac<fNbOfTac; tac++)
	{
		printf("E\t%d\t", tac );
		auto params = getTdcParams_E( absChannelID, tac );
		for( int i=0; i<fNbOfTdcParams; i++ )
		{
			printf("%2.2e\t", params.at(i)); 
		}
		printf("\n");
	}
	return;

}

/// T branch
//double TOF_TdcQdcCalibration::getTime( TOF_TdcQdcCalibration *calp, unsigned int cid, unsigned short tid, long long fid, unsigned short tcoarse, unsigned short tfine )
double TOF_TdcQdcCalibration::getCalibratedTime_T( uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short tcoarse, unsigned short tfine )
{
	//auto chipID    = getChipID   ( absChannelID );
	//auto channelID = getChannelID( absChannelID );
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );

	TOF_Branch br = TOF_Branch::fBranchT;

  double t0 = getT0( chipID, channelID, tacID, br );
  double a0 = getA0( chipID, channelID, tacID, br );
  double a1 = getA1( chipID, channelID, tacID, br );
  double a2 = getA2( chipID, channelID, tacID, br );
  
  float q_T = ( -a1 + sqrtf((a1*a1) - (4.0f * (a0 - tfine) * a2))) / (2.0f * a2);

  return double((frameID*1024+tcoarse)) - q_T - t0;
}

/// E branch
/// this function for E branch is valid when ToT mode
double TOF_TdcQdcCalibration::getCalibratedTime_E( uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short ecoarse, unsigned short efine )
{
	//auto chipID    = getChipID   ( absChannelID );
	//auto channelID = getChannelID( absChannelID );
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );

	TOF_Branch br = TOF_Branch::fBranchE;

  double t0 = getT0( chipID, channelID, tacID, br );
  double a0 = getA0( chipID, channelID, tacID, br );
  double a1 = getA1( chipID, channelID, tacID, br );
  double a2 = getA2( chipID, channelID, tacID, br );
  
  float q_E = ( -a1 + sqrtf((a1*a1) - (4.0f * (a0 - efine) * a2))) / (2.0f * a2);

  return double((frameID*1024+ecoarse)) - q_E - t0;
}

//double getEnergy( TOF_TdcQdcCalibration *calp, unsigned int cid, unsigned short tid, long long fid, unsigned short ecoarse, unsigned short efine, double time )
double TOF_TdcQdcCalibration::getEnergy( uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short ecoarse, unsigned short efine, double time )
{
	//auto chipID    = getChipID   ( absChannelID );
	//auto channelID = getChannelID( absChannelID );
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );
	
  double p0 = getP0( chipID, channelID, tacID );
  double p1 = getP1( chipID, channelID, tacID );
  double p2 = getP2( chipID, channelID, tacID );
  double p3 = getP3( chipID, channelID, tacID );
  double p4 = getP4( chipID, channelID, tacID );
  double p5 = getP5( chipID, channelID, tacID );
  double p6 = getP6( chipID, channelID, tacID );
  double p7 = getP7( chipID, channelID, tacID );
  double p8 = getP8( chipID, channelID, tacID );
  double p9 = getP9( chipID, channelID, tacID );

  //double frequency = 200E6;
  //double Tps = 1E12/frequency;

  double timeEnd = double(frameID*1024+ecoarse);
  double energy  = double(efine);

  float ti = (timeEnd - time);
  float t_eq = ti;
  float delta = 0;
  int iter = 0;

	//std::cout << "timeEnd: " << timeEnd << ", ti: " << ti << std::endl;

  do {
    float f = (p0 - efine) +
      p1 * t_eq + 
      p2 * t_eq * t_eq + 
      p3 * t_eq * t_eq * t_eq + 
      p4 * t_eq * t_eq * t_eq * t_eq +
      p5 * t_eq * t_eq * t_eq * t_eq * t_eq + 
      p6 * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq + 
      p7 * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq + 
      p8 * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq +
      p9 * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq;
    
    float f_ = p1 +
      p2 * t_eq * 2 + 
      p3 * t_eq * t_eq * 3 + 
      p4 * t_eq * t_eq * t_eq * 4 +
      p5 * t_eq * t_eq * t_eq * t_eq * 5 + 
      p6 * t_eq * t_eq * t_eq * t_eq * t_eq * 6 + 
      p7 * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * 7 + 
      p8 * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * 8 +
      p9 * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * t_eq * 9;
    
    delta = - f / f_;
    
    // Avoid very large steps
    if(delta < -10.0) delta = -10.0;
    if(delta > +10.0) delta = +10.0;
    
    t_eq = t_eq + delta;
    iter += 1;
  } while ((fabs(delta) > 0.05) && (iter < 100));
  
  // Express energy as t_eq - actual integration time
  // WARNING Adding 1.0 clock to shift spectrum into positive range
  // .. needs better understanding.
  energy = t_eq - ti;
  
  return energy;
}

//double TOF_TdcQdcCalibration::getCalibratedQDC( uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short ecoarse, unsigned short efine, double time )
double TOF_TdcQdcCalibration::getCalibratedQDC( uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short ecoarse, unsigned short efine, unsigned short tcoarse, double time )
{
	//auto chipID    = getChipID   ( absChannelID );
	//auto channelID = getChannelID( absChannelID );
	auto chipID    = (TOF_ChannelConversion::getInstance())->getAsicID   ( absChannelID );
	auto channelID = (TOF_ChannelConversion::getInstance())->getChannelID( absChannelID );
	
  double p0 = getP0( chipID, channelID, tacID );
  double p1 = getP1( chipID, channelID, tacID );
  double p2 = getP2( chipID, channelID, tacID );
  double p3 = getP3( chipID, channelID, tacID );
  double p4 = getP4( chipID, channelID, tacID );
  double p5 = getP5( chipID, channelID, tacID );
  double p6 = getP6( chipID, channelID, tacID );
  double p7 = getP7( chipID, channelID, tacID );
  double p8 = getP8( chipID, channelID, tacID );
  double p9 = getP9( chipID, channelID, tacID );

	if((ecoarse - tcoarse) < -256) ecoarse += 1024;
  double timeEnd = double(frameID*1024+ecoarse);
  float ti = (timeEnd - time);

	//std::cout << "timeEnd: " << timeEnd << ", ti: " << ti << std::endl;

  float rval = 
		p0 +
    p1 * ti + 
    p2 * ti * ti + 
    p3 * ti * ti * ti + 
    p4 * ti * ti * ti * ti +
    p5 * ti * ti * ti * ti * ti + 
    p6 * ti * ti * ti * ti * ti * ti + 
    p7 * ti * ti * ti * ti * ti * ti * ti + 
    p8 * ti * ti * ti * ti * ti * ti * ti * ti +
    p9 * ti * ti * ti * ti * ti * ti * ti * ti * ti;
    
  return rval; 
}

/*
std::vector<uint8_t> TOF_TdcQdcCalibration::getChipIDChannelID( uint32_t absChannelID )
{
	//uint8_t portID    = (absChannelID >> 17) & 0x1F; 
  //uint8_t slaveID   = (absChannelID >> 12) & 0x1F;
  uint8_t asicID    = (absChannelID >> 6)  & 0x3F; // 0 to 15
  uint8_t channelID = (absChannelID >> 0)  & 0x3F; // 0 to 63

	std::vector<uint8_t> reval = { asicID, channelID };

	return reval;
}

/// chip Id = 0 to 15
uint8_t TOF_TdcQdcCalibration::getChipID( uint32_t absChannelID )
{
	return getChipIDChannelID( absChannelID ).at(0);
}

/// channel ID = 0-63
uint8_t TOF_TdcQdcCalibration::getChannelID( uint32_t absChannelID )
{
	return getChipIDChannelID( absChannelID ).at(1);
}
*/
