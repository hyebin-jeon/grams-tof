#pragma once

#ifndef _TOF_CHANNELCONVERSION_
#define _TOF_CHANNELCONVERSION_

#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <array>
#include "TObject.h"
#include "TString.h"

class TOF_ChannelConversion : public TObject
{
  public:
    inline static TOF_ChannelConversion * theChanCov{nullptr}; 
  	static TOF_ChannelConversion *getInstance() {
  		if( theChanCov == nullptr ) {
  			theChanCov = new TOF_ChannelConversion;
  		}
  
  		return theChanCov;
  	};
  
  	~TOF_ChannelConversion() = default;

	private: 
		/// map between connector ID on FEB/S (J1-J128) and channel ID on the software (ch0-ch127)
		std::map<uint8_t, uint8_t> fMap_ConnIdToChannelId;
		std::map<uint8_t, uint8_t> fMap_ChannelIdToConnId;

		uint8_t fFebD_connID0; // TOF
		uint8_t fFebD_connID1; // MPD

	public:
		void fillMapConnIdToChannelId();
		void fillMapChannelIdToConnId();
		void fillChannelMaps();

		std::map< uint8_t, uint8_t > getMapConnIdToChannelId();
		std::map< uint8_t, uint8_t > getMapChannelIdToConnId();

		/// Conversion from connector IDs (hardware) to channel IDs (software)
		uint8_t  getAsicID( uint8_t febD_connID, uint8_t febS_connID ); // [0-15]
		uint8_t  getChannelID_128( uint8_t febS_connID ); // [0-127] within a set of two ASICs
		uint8_t  getChannelID_64 ( uint8_t febS_connID ); // [0-63] within an ASIC
    uint32_t getAbsoluteChannelID( uint8_t febD_connID, uint8_t febS_connID );
		uint32_t getAbsoluteChannelID( uint8_t portID, uint8_t slaveID, uint8_t chipID, uint8_t channelID );

		/// Break an absolute channel ID into each element
		uint8_t getPortID   ( uint32_t absoluteChannel );
		uint8_t getSlaveID  ( uint32_t absoluteChannel );
		uint8_t getAsicID   ( uint32_t absoluteChannel ); // 0-15
		uint8_t getChannelID( uint32_t absoluteChannel ); // 0-63

		uint8_t getAsicIdx   ( uint32_t absoluteChannel ); // 0 or 1
		uint8_t getChannelID_128( uint32_t absoluteChannel ); // 0-127
		
		/// Conversion from an absolute channel ID (software) to connector IDs (hardware)
		uint8_t getConnIdOnFebD( uint32_t absoluteChannel ); // 1-8
		uint8_t getConnIdOnFebS( uint32_t absoluteChannel ); // 1-128

		int      readActiveAsicList( const char* fname );
		uint16_t getPhysicalChannelID( uint32_t absoluteChannel );
		uint16_t getPhysicalChannelID( uint8_t febD_connID, uint8_t febS_connID );
		uint16_t getPhysicalChannelID( uint8_t portID, uint8_t slaveID, uint8_t chipID, uint8_t channelID );

		void    setActiveConnIdOnFebD( uint8_t febD_connID_a, uint8_t febD_connID_b );
		std::array<uint8_t,2> getActiveConnIdOnFebD() { return {fFebD_connID0, fFebD_connID1}; };
		
	  ClassDef(TOF_ChannelConversion, 1)

};

#endif
