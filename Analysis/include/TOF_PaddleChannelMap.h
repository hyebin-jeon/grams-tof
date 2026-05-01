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

			thePadMap->fillMapConnIdToPaddleId();
			thePadMap->fillMapPaddleIdToConnId();
			thePadMap->fillMapChannelIdAndPaddleId();
  
  		return thePadMap;
  	};
  
  	~TOF_PaddleChannelMap() = default;
	
	public:
		enum eSystem : uint16_t {
			fTopTOF = 0,
			fMidTOF = 1,
			fMPD    = 2,
			fTrig   = 3,
			fPPS    = 4,
		};

	private:
		void fillMapConnIdToPaddleId();
		void fillMapPaddleIdToConnId();
		void fillMapChannelIdAndPaddleId();

		std::map< std::pair<uint8_t, uint8_t> , uint16_t > fMap_ConnIdToPaddleId; // pair<FEB-S Idx, ConnId> --> paddle Idx 0xABBC
		std::map< uint16_t, std::pair<uint8_t, uint8_t> > fMap_PaddleIdToConnId;
		std::map< uint16_t, uint32_t > fMap_PaddleIdToChannelId;
		std::map< uint32_t, uint16_t > fMap_ChannelIdToPaddleId;

	public:
		/// based on the maps
		uint16_t getPaddleId( uint8_t FebS_idx, uint8_t connId );
		uint16_t getPaddleId( uint32_t absChanId );
		uint32_t getAbsoluteChannelId( uint16_t paddleId );
		std::pair<uint8_t, uint8_t> getFebSIdxAndConnId( uint16_t paddleId );

		/// paddleID <-> paddle Indice (system, paddle, channel)
		uint16_t getPaddleId( uint16_t systemIdx, uint16_t paddleIdx, uint16_t channelIdx );
		uint16_t getSystemIdx         ( uint16_t paddleId ){ return (paddleId>>12) & 0xF; };
		uint16_t getPaddleIdxInSystem ( uint16_t paddleId ){ return (paddleId>>4) & 0xFF; };
		uint16_t getChannelIdxInPaddle( uint16_t paddleId ){ return paddleId & 0xF; };
	  
		ClassDef(TOF_PaddleChannelMap, 1)

};

