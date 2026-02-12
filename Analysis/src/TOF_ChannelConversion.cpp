#include "TOF_Constants.h"
#include "TOF_ChannelConversion.h"
#include <cmath>

ClassImp( TOF_ChannelConversion );


/// See 'ChannelMap' sheet at the following link
/// https://docs.google.com/spreadsheets/d/1LlCMwjuo3svoJcFjCKX83WI_OGRFMRbY1DGDkIOTpZs/edit?usp=sharing
void TOF_ChannelConversion::fillMapConnIdToChannelId()
{
	//////////////
	/// ASIC-1 ///
	//////////////

	/// row-1, col-1
	fMap_ConnIdToChannelId[ 105 ] = 89;
	fMap_ConnIdToChannelId[ 106 ] = 88;
	fMap_ConnIdToChannelId[ 107 ] = 90;
	fMap_ConnIdToChannelId[ 108 ] = 92;
	fMap_ConnIdToChannelId[ 109 ] = 93;
	fMap_ConnIdToChannelId[ 110 ] = 94;
	fMap_ConnIdToChannelId[ 111 ] = 95;
	fMap_ConnIdToChannelId[ 112 ] = 96;

  /// row-2, col-1
	fMap_ConnIdToChannelId[  97 ] = 80;
	fMap_ConnIdToChannelId[  98 ] = 82;
	fMap_ConnIdToChannelId[  99 ] = 81;
	fMap_ConnIdToChannelId[ 100 ] = 83;
	fMap_ConnIdToChannelId[ 101 ] = 85;
	fMap_ConnIdToChannelId[ 102 ] = 84;
	fMap_ConnIdToChannelId[ 103 ] = 87;
	fMap_ConnIdToChannelId[ 104 ] = 86;

  /// row-3, col-1
	fMap_ConnIdToChannelId[ 72 ] = 71;
	fMap_ConnIdToChannelId[ 71 ] = 78;
	fMap_ConnIdToChannelId[ 70 ] = 69;
	fMap_ConnIdToChannelId[ 69 ] = 76;
	fMap_ConnIdToChannelId[ 68 ] = 67;
	fMap_ConnIdToChannelId[ 67 ] = 74;
	fMap_ConnIdToChannelId[ 66 ] = 64;
	fMap_ConnIdToChannelId[ 65 ] = 65;

  /// row-4, col-1
	fMap_ConnIdToChannelId[ 73 ] = 72;
	fMap_ConnIdToChannelId[ 74 ] = 79;
	fMap_ConnIdToChannelId[ 75 ] = 70;
	fMap_ConnIdToChannelId[ 76 ] = 68;
	fMap_ConnIdToChannelId[ 77 ] = 77;
	fMap_ConnIdToChannelId[ 78 ] = 66;
	fMap_ConnIdToChannelId[ 79 ] = 75;
	fMap_ConnIdToChannelId[ 80 ] = 91;

	/// row-1, col-2
	fMap_ConnIdToChannelId[ 113 ] = 98;
	fMap_ConnIdToChannelId[ 114 ] = 97;
	fMap_ConnIdToChannelId[ 115 ] = 100;
	fMap_ConnIdToChannelId[ 116 ] = 99;
	fMap_ConnIdToChannelId[ 117 ] = 101;
	fMap_ConnIdToChannelId[ 118 ] = 103;
	fMap_ConnIdToChannelId[ 119 ] = 105;
	fMap_ConnIdToChannelId[ 120 ] = 104;

	/// row-2, col-2
	fMap_ConnIdToChannelId[ 121 ] = 107;
	fMap_ConnIdToChannelId[ 122 ] = 106;
	fMap_ConnIdToChannelId[ 123 ] = 108;
	fMap_ConnIdToChannelId[ 124 ] = 110;
	fMap_ConnIdToChannelId[ 125 ] = 109;
	fMap_ConnIdToChannelId[ 126 ] = 111;
	fMap_ConnIdToChannelId[ 127 ] = 112;
	fMap_ConnIdToChannelId[ 128 ] = 113;

	/// row-3, col-2
	fMap_ConnIdToChannelId[ 96 ] = 127;
	fMap_ConnIdToChannelId[ 95 ] = 124;
	fMap_ConnIdToChannelId[ 94 ] = 119;
	fMap_ConnIdToChannelId[ 93 ] = 120;
	fMap_ConnIdToChannelId[ 92 ] = 117;
	fMap_ConnIdToChannelId[ 91 ] = 118;
	fMap_ConnIdToChannelId[ 90 ] = 115;
	fMap_ConnIdToChannelId[ 89 ] = 116;

	/// row-4, col-2
	fMap_ConnIdToChannelId[ 81 ] = 102;
	fMap_ConnIdToChannelId[ 82 ] =  73;
	fMap_ConnIdToChannelId[ 83 ] = 126;
	fMap_ConnIdToChannelId[ 84 ] = 122;
	fMap_ConnIdToChannelId[ 85 ] = 125;
	fMap_ConnIdToChannelId[ 86 ] = 123;
	fMap_ConnIdToChannelId[ 87 ] = 114;
	fMap_ConnIdToChannelId[ 88 ] = 121;
	
	//////////////
	/// ASIC-0 ///
	//////////////

	/// row-1, col-1
	fMap_ConnIdToChannelId[ 24 ] = 57;
	fMap_ConnIdToChannelId[ 23 ] = 50;
	fMap_ConnIdToChannelId[ 22 ] = 59;
	fMap_ConnIdToChannelId[ 21 ] = 61;
	fMap_ConnIdToChannelId[ 20 ] = 58;
	fMap_ConnIdToChannelId[ 19 ] = 62;
	fMap_ConnIdToChannelId[ 18 ] =  9;
	fMap_ConnIdToChannelId[ 17 ] = 38;

	/// row-2, col-1
	fMap_ConnIdToChannelId[ 25 ] = 52;
	fMap_ConnIdToChannelId[ 26 ] = 51; 
	fMap_ConnIdToChannelId[ 27 ] = 54;
	fMap_ConnIdToChannelId[ 28 ] = 53;
	fMap_ConnIdToChannelId[ 29 ] = 56;
	fMap_ConnIdToChannelId[ 30 ] = 55;
	fMap_ConnIdToChannelId[ 31 ] = 60;
	fMap_ConnIdToChannelId[ 32 ] = 63;

	/// row-3, col-1
	fMap_ConnIdToChannelId[ 64 ] = 49;
	fMap_ConnIdToChannelId[ 63 ] = 48;
	fMap_ConnIdToChannelId[ 62 ] = 47;
	fMap_ConnIdToChannelId[ 61 ] = 45;
	fMap_ConnIdToChannelId[ 60 ] = 46;
	fMap_ConnIdToChannelId[ 59 ] = 44;
	fMap_ConnIdToChannelId[ 58 ] = 42;
	fMap_ConnIdToChannelId[ 57 ] = 43;

	/// row-4, col-1
	fMap_ConnIdToChannelId[ 56 ] = 40;
	fMap_ConnIdToChannelId[ 55 ] = 41;
	fMap_ConnIdToChannelId[ 54 ] = 39;
	fMap_ConnIdToChannelId[ 53 ] = 37;
	fMap_ConnIdToChannelId[ 52 ] = 35;
	fMap_ConnIdToChannelId[ 51 ] = 36;
	fMap_ConnIdToChannelId[ 50 ] = 33;
	fMap_ConnIdToChannelId[ 49 ] = 34;

	/// row-1, col-2
	fMap_ConnIdToChannelId[ 16 ] = 27;
	fMap_ConnIdToChannelId[ 15 ] = 11;
	fMap_ConnIdToChannelId[ 14 ] =  2;
	fMap_ConnIdToChannelId[ 13 ] = 13;
	fMap_ConnIdToChannelId[ 12 ] =  4;
	fMap_ConnIdToChannelId[ 11 ] =  6;
	fMap_ConnIdToChannelId[ 10 ] = 15;
	fMap_ConnIdToChannelId[  9 ] =  8;

	/// row-2, col-2
	fMap_ConnIdToChannelId[ 1 ] =  1;
	fMap_ConnIdToChannelId[ 2 ] =  0;
	fMap_ConnIdToChannelId[ 3 ] = 10;
	fMap_ConnIdToChannelId[ 4 ] =  3;
	fMap_ConnIdToChannelId[ 5 ] = 12;
	fMap_ConnIdToChannelId[ 6 ] =  5;
	fMap_ConnIdToChannelId[ 7 ] = 14;
	fMap_ConnIdToChannelId[ 8 ] =  7;

	/// row-3, col-2
	fMap_ConnIdToChannelId[ 40 ] = 22;
	fMap_ConnIdToChannelId[ 39 ] = 23;
	fMap_ConnIdToChannelId[ 38 ] = 20;
	fMap_ConnIdToChannelId[ 37 ] = 21;
	fMap_ConnIdToChannelId[ 36 ] = 19;
	fMap_ConnIdToChannelId[ 35 ] = 17;
	fMap_ConnIdToChannelId[ 34 ] = 18;
	fMap_ConnIdToChannelId[ 33 ] = 16;

	/// row-3, col-2
	fMap_ConnIdToChannelId[ 48 ] = 32;
	fMap_ConnIdToChannelId[ 47 ] = 31;
	fMap_ConnIdToChannelId[ 46 ] = 30;
	fMap_ConnIdToChannelId[ 45 ] = 29;
	fMap_ConnIdToChannelId[ 44 ] = 28;
	fMap_ConnIdToChannelId[ 43 ] = 26;
	fMap_ConnIdToChannelId[ 42 ] = 24;
	fMap_ConnIdToChannelId[ 41 ] = 25;

	return;
}

		
void TOF_ChannelConversion::fillMapChannelIdToConnId()
{
	if( fMap_ConnIdToChannelId.size() != 128 ) fillMapConnIdToChannelId(); 
	for( int i=0; i<128; i++ )
	{
		uint8_t connID = i+1;
		uint8_t channel = fMap_ConnIdToChannelId[ connID ];

		fMap_ChannelIdToConnId[ channel ] = connID;
	}

	return;
}
		
void TOF_ChannelConversion::fillChannelMaps()
{
	fillMapConnIdToChannelId();
	fillMapChannelIdToConnId();

	return;
}

std::map<uint8_t, uint8_t> TOF_ChannelConversion::getMapConnIdToChannelId()
{
	if( fMap_ConnIdToChannelId.size() != 128 ) fillMapConnIdToChannelId();

	return fMap_ConnIdToChannelId;
}

std::map<uint8_t, uint8_t> TOF_ChannelConversion::getMapChannelIdToConnId()
{
	if( fMap_ChannelIdToConnId.size() != 128 ) fillMapChannelIdToConnId();

	return fMap_ChannelIdToConnId;
}

uint8_t TOF_ChannelConversion::getChannelID_128( uint8_t febS_connID )
{
	if( fMap_ConnIdToChannelId.size() != 128 ) fillMapConnIdToChannelId();

	return fMap_ConnIdToChannelId[ febS_connID ];
}

uint8_t TOF_ChannelConversion::getChannelID_64( uint8_t febS_connID )
{
	if( fMap_ConnIdToChannelId.size() != 128 ) fillMapConnIdToChannelId();

	uint8_t channel128 = getChannelID_128( febS_connID );

	return channel128%64;
}

uint8_t TOF_ChannelConversion::getAsicID( uint8_t febD_connID, uint8_t febS_connID )
{
	if( febD_connID<  1 ) return TOF_ERR;
	if( febD_connID> 15 ) return TOF_ERR;
	if( febS_connID<  1 ) return TOF_ERR;
	if( febS_connID> 128) return TOF_ERR;

	uint8_t asicID0 = (febD_connID-1)*2; 
	uint8_t chanID0 = getChannelID_128( febS_connID );
	uint8_t asicID  = asicID0 + chanID0/64;

	return asicID;
}
	
uint32_t TOF_ChannelConversion::getAbsoluteChannelID( uint8_t febD_connID, uint8_t febS_connID )
{
	uint8_t portID  = 0; // DAQ portID
	uint8_t slaveID = 0; // ID used for daisy chain of FEB/D
	uint8_t asicID  = getAsicID( febD_connID, febS_connID );
	uint8_t chanID  = getChannelID_64( febS_connID );

	/// According to the user manual, TOFPET2 ASIC SIPM Readout System Software user guide,
	/// absolute channel ID = 131072*portID + 4096*slaveID + 64*chipID + ChannelID
	uint32_t absChanID = pow(2, 17) * static_cast<uint32_t>(portID )
		                 + pow(2, 12) * static_cast<uint32_t>(slaveID)
		                 + pow(2,  6) * static_cast<uint32_t>(asicID )
	                   + static_cast<uint32_t>(chanID);

	//cout << Form("portID= %2d, slaveID= %2d, asicID0= %2d, asicID1= %2d, chanID= %2d ==> Abs. channelID = %7d", portID, slaveID, asicID0, asicID1, chanID, absChanID) << endl;

	return absChanID;
}

uint32_t TOF_ChannelConversion::getAbsoluteChannelID( uint8_t portID, uint8_t slaveID, uint8_t chipID, uint8_t channelID )
{
	return 131072* static_cast<uint32_t>(portID) + 4096* static_cast<uint32_t>(slaveID) + 64*static_cast<uint32_t>(chipID) + static_cast<uint32_t>(channelID);
}

		
uint8_t TOF_ChannelConversion::getPortID( uint32_t channel )
{
	uint8_t portID = (channel>>17) & 0x1F;
	return portID;
}
uint8_t TOF_ChannelConversion::getSlaveID( uint32_t channel )
{
	uint8_t slaveID = (channel>>12) & 0x1F;
	return slaveID;
}
uint8_t TOF_ChannelConversion::getAsicID( uint32_t channel )
{
	auto asicID = (channel>>6) & 0x3F;
	return asicID;
}
uint8_t TOF_ChannelConversion::getChannelID( uint32_t channel )
{
	auto chanID = (channel>>0) & 0x3F;
	return chanID;
}

uint8_t TOF_ChannelConversion::getAsicIdx( uint32_t channel )
{
	auto asicID = getAsicID( channel );
	return asicID%2;
}
uint8_t TOF_ChannelConversion::getChannelID_128( uint32_t channel )
{
	auto asicIdx = getAsicIdx( channel );
	auto chanID = getChannelID( channel );

	return asicIdx*64 + chanID;
}

uint8_t TOF_ChannelConversion::getConnIdOnFebS( uint32_t channel )
{
	auto chanID = getChannelID_128( channel );

	if( fMap_ChannelIdToConnId.size() != 128 ) fillMapChannelIdToConnId();

	return fMap_ChannelIdToConnId[ chanID ];
}

uint8_t TOF_ChannelConversion::getConnIdOnFebD( uint32_t channel )
{
	uint8_t asicID = getAsicID( channel );
	uint8_t connID = 2 * (asicID/2) + 1;

	return connID;
}

uint16_t TOF_ChannelConversion::getPhysicalChannelID( uint32_t absoluteChannel )
{
	auto febD = getConnIdOnFebD( absoluteChannel );
	auto febS = getConnIdOnFebS( absoluteChannel );

	uint8_t febD_idx = 2; // dummy initilization
	if     ( febD == fFebD_connID0 ) febD_idx = 0;
	else if( febD == fFebD_connID1 ) febD_idx = 1;
	else std::cerr << "[WARN] This channel is not connected to the active FEB-D connector" << std::endl;

	uint16_t phyID = static_cast<uint16_t>( febD ) * 128 + febS;

	return phyID;
}

uint16_t TOF_ChannelConversion::getPhysicalChannelID( uint8_t febD, uint8_t febS )
{
	uint8_t febD_idx = 2; // dummy initilization
	if     ( febD == fFebD_connID0 ) febD_idx = 0;
	else if( febD == fFebD_connID1 ) febD_idx = 1;
	else std::cerr << "[WARN] This channel is not connected to the active FEB-D connector" << std::endl;

	uint16_t phyID = static_cast<uint16_t>( febD ) * 128 + febS;

	return phyID;
}

uint16_t TOF_ChannelConversion::getPhysicalChannelID( uint8_t portID, uint8_t slaveID, uint8_t chipID, uint8_t channelID )
{
	auto absoluteChannel = getAbsoluteChannelID( portID, slaveID, chipID, channelID );
	auto febD = getConnIdOnFebD( absoluteChannel );
	auto febS = getConnIdOnFebS( absoluteChannel );

	uint8_t febD_idx = 2; // dummy initilization
	if     ( febD == fFebD_connID0 ) febD_idx = 0;
	else if( febD == fFebD_connID1 ) febD_idx = 1;
	else std::cerr << "[WARN] This channel is not connected to the active FEB-D connector" << std::endl;

	uint16_t phyID = static_cast<uint16_t>( febD ) * 128 + febS;

	return phyID;
}

