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
#include "TOF_Constants.h"

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
  
  		return thePadMap;
  	};
  
  	~TOF_PaddleChannelMap() = default;
	
	private:
		int  fVerbose{0};
		void fillMapConnIdToPaddleId();
		void fillMapPaddleIdToConnId();
		void fillMapChannelIdAndPaddleId();

		std::map< std::pair<uint8_t, uint8_t> , uint16_t > fMap_ConnIdToPaddleIdx; // pair<FEB-S Idx, ConnIdOnFebS> --> paddle Idx 0xABBC
		std::map< uint16_t, std::pair<uint8_t, uint8_t> > fMap_PaddleIdxToConnIDs; // paddle Idx 0xABBC--> pair<FEB-S Idx, ConnIdOnFebS>
	private:
		/// paddleID <-> paddle Indice (system, paddle, channel)
		uint16_t getPaddleIdx( uint16_t systemIdx, uint16_t paddleLocId, uint16_t sipmLocId );

	public:
		int     getFebSIdx( uint8_t connIdOnFebD );
		uint8_t getSystemIdx  ( uint16_t paddleIdx ){ return (paddleIdx>>12) & 0xF ; };
		uint8_t getPaddleLocId( uint16_t paddleIdx ){ return (paddleIdx>> 4) & 0xFF; };
		uint8_t getSipmLocId  ( uint16_t paddleIdx ){ return paddleIdx       & 0xF ; };
	
	public:
		uint16_t getPaddleIdx  ( uint8_t connID_D, uint8_t connID_S );
		uint8_t  getSystemIdx  ( uint8_t connID_D, uint8_t connID_S );
		uint8_t  getPaddleLocId( uint8_t connID_D, uint8_t connID_S );
		uint8_t  getSipmLocId  ( uint8_t connID_D, uint8_t connID_S );

		//bool isTriggerChannel( uint8_t connID_D, uint8_t connID_S );
		//bool isPpsChannel    ( uint8_t connID_D, uint8_t connID_S );
		//bool isTestPaddle    ( uint8_t connID_D, uint8_t connID_S );
		bool isTriggerChannel( uint16_t paddleIdx );
		bool isPpsChannel    ( uint16_t paddleIdx );
		bool isTestPaddle    ( uint16_t paddleIdx );
		
		void dump();
	  
		ClassDef(TOF_PaddleChannelMap, 1)

};

