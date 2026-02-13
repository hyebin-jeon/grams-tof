#pragma once

#ifndef _TOF_TDCQDCCALIBRATION_
#define _TOF_TDCQDCCALIBRATION_

#include <iostream>
#include <fstream>
#include "TObject.h"
#include "TString.h"
#include "TOF_Constants.h"
//#include "TOF_TdcQdcCalibration.h"
#include "TOF_ChannelConversion.h"


class TOF_TdcQdcCalibration : public TObject
{
	public:
    TOF_TdcQdcCalibration() { 
			initializeParams(); 
		};
    TOF_TdcQdcCalibration( const char* fTdcCalib, const char* fQdcCalib )
		{
			initializeParams(); 
      readTdcCalib( fTdcCalib );
      readQdcCalib( fQdcCalib );
		}

    inline static TOF_TdcQdcCalibration * theCalib{nullptr}; 
		static TOF_TdcQdcCalibration *getInstance() {
			if( theCalib == nullptr ) {
				theCalib = new TOF_TdcQdcCalibration;
			}
			return theCalib;
		}

    ~TOF_TdcQdcCalibration() = default;

	private:
		const int fNbOfTdcParams {4};
		const int fNbOfQdcParams {10};
		const int fNbOfTac {4}; // in a single channel

	public:
		void initializeParams();
	  /// QDC calibration parameters
    inline double getP0( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P0[chipID][channelID][tacID]; };
    inline double getP1( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P1[chipID][channelID][tacID]; };
    inline double getP2( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P2[chipID][channelID][tacID]; };
    inline double getP3( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P3[chipID][channelID][tacID]; };
    inline double getP4( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P4[chipID][channelID][tacID]; };
    inline double getP5( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P5[chipID][channelID][tacID]; };
    inline double getP6( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P6[chipID][channelID][tacID]; };
    inline double getP7( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P7[chipID][channelID][tacID]; };
    inline double getP8( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P8[chipID][channelID][tacID]; };
    inline double getP9( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return P9[chipID][channelID][tacID]; };
		std::vector<double> getQdcParams( uint32_t absChannelID, uint8_t tacID );

 
	  /// TDC calibration parameters. 
	  /// T branch params when isT = kTRUE, else E branch. isT is fixed to kTRUE
    inline double getT0_T( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return T0[chipID][channelID][tacID][0]; };
    inline double getA0_T( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return A0[chipID][channelID][tacID][0]; };
    inline double getA1_T( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return A1[chipID][channelID][tacID][0]; };
    inline double getA2_T( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return A2[chipID][channelID][tacID][0]; };
    inline double getT0_E( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return T0[chipID][channelID][tacID][1]; };
    inline double getA0_E( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return A0[chipID][channelID][tacID][1]; };
    inline double getA1_E( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return A1[chipID][channelID][tacID][1]; };
    inline double getA2_E( uint8_t chipID, uint32_t channelID, uint8_t tacID ) { return A2[chipID][channelID][tacID][1]; };
    double getT0( uint8_t chipID, uint32_t channelID, uint8_t tacID, TOF_Branch br );
    double getA0( uint8_t chipID, uint32_t channelID, uint8_t tacID, TOF_Branch br );
    double getA1( uint8_t chipID, uint32_t channelID, uint8_t tacID, TOF_Branch br );
    double getA2( uint8_t chipID, uint32_t channelID, uint8_t tacID, TOF_Branch br );
		std::vector<double> getTdcParams_T( uint32_t absChannelID, uint8_t tacID );
		std::vector<double> getTdcParams_E( uint32_t absChannelID, uint8_t tacID );
		std::vector<double> getTdcParams( uint32_t absChannelID, uint8_t tacID );
		

    int readTdcCalib( const char *fname );
    int readQdcCalib( const char *fname );
		int readCalibrationFiles( const char* fTdcCalib, const char* fQdcCalib );
		int readCalibrationFiles( const char* dirPath );
		void printTdcCalibTable( uint32_t absChannelID );
		void printQdcCalibTable( uint32_t absChannelID );
		void printCalibTable( uint32_t absChannelID );

	public:
		//std::vector<uint8_t> getChipIDChannelID( uint32_t absChannelID );
		//uint8_t getChipID( uint32_t absChannelID ); // 0 to 15
		//uint8_t getChannelID( uint32_t absChannelID ); // 0 to 63


	public:
		double getCalibratedTime_T( uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short tcoarse, unsigned short tfine );
    double getCalibratedTime_E( uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short ecoarse, unsigned short efine );
    double getEnergy( uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short ecoarse, unsigned short efine, double time ); 
    double getCalibratedQDC( uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short ecoarse, unsigned short efine,  unsigned short tcoarse, double time ); 
    
		double getCalibratedTime( TOF_Branch branchMode, uint32_t absChannelID, uint8_t tacID, long long frameID, unsigned short coarse, unsigned short fine )
		{
			if( branchMode == TOF_Branch::fBranchT )
				return getCalibratedTime_T(absChannelID, tacID, frameID, coarse, fine );
			else if ( branchMode == TOF_Branch::fBranchE )
			  return getCalibratedTime_E(absChannelID, tacID, frameID, coarse, fine );
			else 
			{
				return TOF_ERR;
			}
		}

	private:
	  /// param[chipID][channelID][tacID][T or E]
    double T0[16][64][4][2];
    double A0[16][64][4][2];
    double A1[16][64][4][2];
    double A2[16][64][4][2];
    
	  /// param[chipID][channelID][tacID]
    double P0[16][64][4];
    double P1[16][64][4];
    double P2[16][64][4];
    double P3[16][64][4];
    double P4[16][64][4];
    double P5[16][64][4];
    double P6[16][64][4];
    double P7[16][64][4];
    double P8[16][64][4];
    double P9[16][64][4];
	
	ClassDef(TOF_TdcQdcCalibration, 1)
};

#endif
