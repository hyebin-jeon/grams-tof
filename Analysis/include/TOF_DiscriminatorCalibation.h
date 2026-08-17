#pragma once

#ifndef _TOF_DISCRIMINATORCALIBRATION_
#define _TOF_DISCRIMINATORCALIBRATION_

#include <iostream>
#include <fstream>
#include "TObject.h"
#include "TString.h"
#include "TOF_Constants.h"
#include "TOF_ChannelConversion.h"


class TOF_DiscriminatorCalibration : public TObject
{
	public:
    TOF_DiscriminatorCalibration() { 
			initializeParams(); 
		};
    TOF_DiscriminatorCalibration( const char* fCalibFile )
		{
			initializeParams(); 
      readCalib( fCalibFile );
		}

    inline static TOF_DiscriminatorCalibration * theCalib{nullptr}; 
		static TOF_DiscriminatorCalibration *getInstance() {
			if( theCalib == nullptr ) {
				theCalib = new TOF_DiscriminatorCalibration;
			}
			return theCalib;
		}

    ~TOF_DiscriminatorCalibration() = default;

	private:
		const int fNbOfDiscrimators {3};

		const int fbranchT = 0;
		const int fbranchE = 1;
		const int fDiscrT1 = 0;
		const int fDiscrT2 = 1;
		const int fDiscrE  = 2;

	private:
	  /// param[chipID][channelID][tacID][T or E]
    int    fBaseline[16][64][2]; // baseline for T and E branches
    double fZero    [16][64][3]; // zero  for t1, t2, e discriminators
    double fNoise   [16][64][3]; // noise for t1, t2, e discriminators
    
	public:
		void initializeParams();
    
		inline int    getBaseline_T( uint8_t chipID, uint32_t channelID ) { return fBaseline[chipID][channelID][fbranchT];};
		inline int    getBaseline_E( uint8_t chipID, uint32_t channelID ) { return fBaseline[chipID][channelID][fbranchE];};
		inline double getZero_t1   ( uint8_t chipID, uint32_t channelID ) { return fZero[chipID][channelID][fDiscrT1];    };
		inline double getZero_t2   ( uint8_t chipID, uint32_t channelID ) { return fZero[chipID][channelID][fDiscrT2];    };
		inline double getZero_e    ( uint8_t chipID, uint32_t channelID ) { return fZero[chipID][channelID][fDiscrE];     };
    inline double getNoise_t1  ( uint8_t chipID, uint32_t channelID ) { return fNoise[chipID][channelID][fDiscrT1];   };
    inline double getNoise_t2  ( uint8_t chipID, uint32_t channelID ) { return fNoise[chipID][channelID][fDiscrT2];   };
    inline double getNoise_e   ( uint8_t chipID, uint32_t channelID ) { return fNoise[chipID][channelID][fDiscrE];    };

		int    getBaseline( uint8_t chipID, uint32_t channelID, TOF_Branch br );
		double getZero    ( uint8_t chipID, uint32_t channelID, TOF_Discriminator disc );
		double getNoise   ( uint8_t chipID, uint32_t channelID, TOF_Discriminator disc );

		std::vector<double> getDiscrParams( uint32_t absChannelID, TOF_Discriminator disc );
    
		int readCalib( const char *fname );
		int readCalibFromDir( const char* dirPath );

		void printCalibTable( uint32_t absChannelID );
	
	ClassDef(TOF_DiscriminatorCalibration, 1)
};

#endif
