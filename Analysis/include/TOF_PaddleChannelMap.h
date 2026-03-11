#pragma once

#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <array>
#include "TObject.h"
#include "TString.h"
#include "TOF_ChannelConversion.h"

/// paddle ID = 0xABBC
/// A: System Idx, B: Paddle Idx, C: Channel Idx within a paddle

class TOF_PaddleChannelMap : public TObject
{
  public:
    inline static TOF_PaddleChannelMap * thePadMap{nullptr}; 
  	static TOF_PaddleChannelMap *getInstance() {
  		if( thePadMap == nullptr ) {
  			thePadMap = new TOF_PaddleChannelMap;
  		}

			thePadMap->fillMapConnIdToPaddle();
			thePadMap->fillMapPaddleToConnId();
			thePadMap->fillMapChannelIdPaddle();
  
  		return thePadMap;
  	};
  
  	~TOF_PaddleChannelMap() = default;

	private:
		//struct sPaddleID_sub{
		//	uint8_t systemIdx;
		//	uint8_t paddleIdx;
		//	uint8_t channelIdx;
		//};

		enum eSystem : uint16_t {
			fTopTOF = 0,
			fMidTOF = 1,
			fMPD    = 2,
			fTrig   = 3,
			fPPS    = 4,
		};
		void fillMapConnIdToPaddle();
		void fillMapPaddleToConnId();
		void fillMapChannelIdPaddle();

		std::map< std::pair<uint8_t, uint8_t> , uint16_t > fMap_ConnIdToPaddle; // pair<FEB-S Id, ConnId> --> paddle ID 0xABBC
		std::map< uint16_t, std::pair<uint8_t, uint8_t> > fMap_PaddleToConnId;
		std::map< uint16_t, uint32_t > fMap_PaddleToChannelId;
		std::map< uint32_t, uint16_t > fMap_ChannelIdToPaddle;

		uint16_t getPaddleId( uint16_t systemIdx, uint16_t paddleIdx, uint16_t channelIdx );
	  
		ClassDef(TOF_PaddleChannelMap, 1)

};

