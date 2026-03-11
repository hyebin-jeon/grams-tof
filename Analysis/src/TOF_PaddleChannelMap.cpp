#include "TOF_PaddleChannelMap.h"

uint16_t TOF_PaddleChannelMap::getPaddleId( uint16_t systemIdx, uint16_t paddleIdx, uint16_t channelIdx )
{
	return (systemIdx<< 4*3) + (paddleIdx<<4) + channelIdx;
}

void TOF_PaddleChannelMap::fillMapConnIdToPaddle()
{
	//////////////////////////////////////////
	/// FEB-S #0
	//////////////////////////////////////////

	//////////////
	/// ASIC-1 ///
	//////////////

	/// row-1, col-1
	fMap_ConnIdToPaddle[ std::make_pair(0,105) ] = getPaddleId( fTopTOF, 1, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,106) ] = getPaddleId( fTopTOF, 2, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,107) ] = getPaddleId( fTopTOF, 3, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,108) ] = getPaddleId( fTopTOF, 4, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,109) ] = getPaddleId( fTopTOF, 5, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,110) ] = getPaddleId( fTopTOF, 6, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,111) ] = getPaddleId( fTopTOF, 7, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,112) ] = getPaddleId( fTopTOF, 8, 0);

  /// row-2, col-1
	fMap_ConnIdToPaddle[ std::make_pair(0, 97) ] = getPaddleId( fTopTOF,  9, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0, 98) ] = getPaddleId( fTopTOF, 10, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0, 99) ] = getPaddleId( fTopTOF, 11, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,100) ] = getPaddleId( fTopTOF, 12, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,101) ] = getPaddleId( fTopTOF, 13, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,102) ] = getPaddleId( fTopTOF, 14, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,103) ] = getPaddleId( fTopTOF, 15, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,104) ] = getPaddleId( fTopTOF, 16, 0);

  /// row-3, col-1
	fMap_ConnIdToPaddle[ std::make_pair(0,72) ] = getPaddleId( fTopTOF, 17, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,71) ] = getPaddleId( fTopTOF, 18, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,70) ] = getPaddleId( fTopTOF, 19, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,69) ] = getPaddleId( fTopTOF, 20, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,68) ] = getPaddleId( fTopTOF, 21, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,67) ] = getPaddleId( fTopTOF, 22, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,66) ] = getPaddleId( fTopTOF, 23, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,65) ] = getPaddleId( fTopTOF, 24, 0);

  /// row-4, col-1
	fMap_ConnIdToPaddle[ std::make_pair(0,73) ] = getPaddleId( fMidTOF, 1, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,74) ] = getPaddleId( fMidTOF, 2, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,75) ] = getPaddleId( fMidTOF, 3, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,76) ] = getPaddleId( fMidTOF, 4, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,77) ] = getPaddleId( fMidTOF, 5, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,78) ] = getPaddleId( fMidTOF, 6, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,79) ] = getPaddleId( fMidTOF, 7, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,80) ] = getPaddleId( fMidTOF, 8, 0);

	/// row-1, col-2
	fMap_ConnIdToPaddle[ std::make_pair(0,113) ] = getPaddleId( fMidTOF,  9, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,114) ] = getPaddleId( fMidTOF, 10, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,115) ] = getPaddleId( fMidTOF, 11, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,116) ] = getPaddleId( fMidTOF, 12, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,117) ] = getPaddleId( fMidTOF, 13, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,118) ] = getPaddleId( fMidTOF, 14, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,119) ] = getPaddleId( fMidTOF, 15, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,120) ] = getPaddleId( fMidTOF, 16, 0);

	/// row-2, col-2
	fMap_ConnIdToPaddle[ std::make_pair(0,121) ] = getPaddleId( fMPD, 1, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,122) ] = getPaddleId( fMPD, 1, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,123) ] = getPaddleId( fMPD, 1, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,124) ] = getPaddleId( fMPD, 2, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,125) ] = getPaddleId( fMPD, 2, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,126) ] = getPaddleId( fMPD, 2, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,127) ] = getPaddleId( fMPD, 3, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,128) ] = getPaddleId( fMPD, 3, 1);

	/// row-3, col-2
	fMap_ConnIdToPaddle[ std::make_pair(0,96) ] = getPaddleId( fMPD, 3, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,95) ] = getPaddleId( fMPD, 4, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,94) ] = getPaddleId( fMPD, 4, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,93) ] = getPaddleId( fMPD, 4, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,92) ] = getPaddleId( fMPD, 5, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,91) ] = getPaddleId( fMPD, 5, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,90) ] = getPaddleId( fMPD, 5, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,89) ] = getPaddleId( fMPD, 6, 0);

	/// row-4, col-2
	fMap_ConnIdToPaddle[ std::make_pair(0,81) ] = getPaddleId( fMPD, 6, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,82) ] = getPaddleId( fMPD, 6, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,83) ] = getPaddleId( fMPD, 7, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,84) ] = getPaddleId( fMPD, 7, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,85) ] = getPaddleId( fMPD, 7, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,86) ] = getPaddleId( fMPD, 8, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,87) ] = getPaddleId( fMPD, 8, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,88) ] = getPaddleId( fMPD, 8, 2);
	
	//////////////
	/// ASIC-0 ///
	//////////////

	/// row-1, col-1
	fMap_ConnIdToPaddle[ std::make_pair(0,24) ] = getPaddleId( fMPD,  9, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,23) ] = getPaddleId( fMPD,  9, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,22) ] = getPaddleId( fMPD,  9, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,21) ] = getPaddleId( fMPD, 10, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,20) ] = getPaddleId( fMPD, 10, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,19) ] = getPaddleId( fMPD, 10, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,18) ] = getPaddleId( fMPD, 11, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,17) ] = getPaddleId( fMPD, 11, 1);

	/// row-2, col-1
	fMap_ConnIdToPaddle[ std::make_pair(0,25) ] = getPaddleId( fMPD, 11, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,26) ] = getPaddleId( fMPD, 12, 0); 
	fMap_ConnIdToPaddle[ std::make_pair(0,27) ] = getPaddleId( fMPD, 12, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,28) ] = getPaddleId( fMPD, 12, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,29) ] = getPaddleId( fMPD, 13, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,30) ] = getPaddleId( fMPD, 13, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,31) ] = getPaddleId( fMPD, 13, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,32) ] = getPaddleId( fMPD, 14, 0);

	/// row-3, col-1
	fMap_ConnIdToPaddle[ std::make_pair(0,64) ] = getPaddleId( fMPD, 14, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,63) ] = getPaddleId( fMPD, 14, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,62) ] = getPaddleId( fMPD, 15, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,61) ] = getPaddleId( fMPD, 15, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,60) ] = getPaddleId( fMPD, 15, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,59) ] = getPaddleId( fMPD, 16, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,58) ] = getPaddleId( fMPD, 16, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,57) ] = getPaddleId( fMPD, 16, 2);

	/// row-4, col-1
	fMap_ConnIdToPaddle[ std::make_pair(0,56) ] = getPaddleId( fMPD, 17, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,55) ] = getPaddleId( fMPD, 17, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,54) ] = getPaddleId( fMPD, 17, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,53) ] = getPaddleId( fMPD, 18, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,52) ] = getPaddleId( fMPD, 18, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,51) ] = getPaddleId( fMPD, 18, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,50) ] = getPaddleId( fMPD, 19, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,49) ] = getPaddleId( fMPD, 19, 1);

	/// row-1, col-2
	fMap_ConnIdToPaddle[ std::make_pair(0,16) ] = getPaddleId( fMPD, 19, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,15) ] = getPaddleId( fMPD, 20, 0);
	fMap_ConnIdToPaddle[ std::make_pair(0,14) ] = getPaddleId( fMPD, 20, 1);
	fMap_ConnIdToPaddle[ std::make_pair(0,13) ] = getPaddleId( fMPD, 20, 2);
	fMap_ConnIdToPaddle[ std::make_pair(0,12) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,11) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,10) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0, 9) ] = 0xFFFF;

	/// row-2, col-2
	fMap_ConnIdToPaddle[ std::make_pair(0,1) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,2) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,3) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,4) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,5) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,6) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,7) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,8) ] = 0xFFFF;

	/// row-3, col-2
	fMap_ConnIdToPaddle[ std::make_pair(0,40) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,39) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,38) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,37) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,36) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,35) ] = getPaddleId( fTrig, 0, 0 );
	fMap_ConnIdToPaddle[ std::make_pair(0,34) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,33) ] = 0xFFFF;

	/// row-3, col-2
	fMap_ConnIdToPaddle[ std::make_pair(0,48) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,47) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,46) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,45) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,44) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,43) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,42) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(0,41) ] = 0xFFFF;


	//////////////////////////////////////////
	/// FEB-S #1
	//////////////////////////////////////////
	
	//////////////
	/// ASIC-1 ///
	//////////////

	/// row-1, col-1
	fMap_ConnIdToPaddle[ std::make_pair(1,105) ] = getPaddleId( fTopTOF, 1, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,106) ] = getPaddleId( fTopTOF, 2, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,107) ] = getPaddleId( fTopTOF, 3, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,108) ] = getPaddleId( fTopTOF, 4, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,109) ] = getPaddleId( fTopTOF, 5, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,110) ] = getPaddleId( fTopTOF, 6, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,111) ] = getPaddleId( fTopTOF, 7, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,112) ] = getPaddleId( fTopTOF, 8, 0);

  /// row-2, col-1
	fMap_ConnIdToPaddle[ std::make_pair(1, 97) ] = getPaddleId( fTopTOF,  9, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1, 98) ] = getPaddleId( fTopTOF, 10, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1, 99) ] = getPaddleId( fTopTOF, 11, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,100) ] = getPaddleId( fTopTOF, 12, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,101) ] = getPaddleId( fTopTOF, 13, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,102) ] = getPaddleId( fTopTOF, 14, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,103) ] = getPaddleId( fTopTOF, 15, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,104) ] = getPaddleId( fTopTOF, 16, 0);

  /// row-3, col-1
	fMap_ConnIdToPaddle[ std::make_pair(1,72) ] = getPaddleId( fTopTOF, 17, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,71) ] = getPaddleId( fTopTOF, 18, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,70) ] = getPaddleId( fTopTOF, 19, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,69) ] = getPaddleId( fTopTOF, 20, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,68) ] = getPaddleId( fTopTOF, 21, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,67) ] = getPaddleId( fTopTOF, 22, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,66) ] = getPaddleId( fTopTOF, 23, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,65) ] = getPaddleId( fTopTOF, 24, 0);

  /// row-4, col-1
	fMap_ConnIdToPaddle[ std::make_pair(1,73) ] = getPaddleId( fMidTOF, 1, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,74) ] = getPaddleId( fMidTOF, 2, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,75) ] = getPaddleId( fMidTOF, 3, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,76) ] = getPaddleId( fMidTOF, 4, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,77) ] = getPaddleId( fMidTOF, 5, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,78) ] = getPaddleId( fMidTOF, 6, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,79) ] = getPaddleId( fMidTOF, 7, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,80) ] = getPaddleId( fMidTOF, 8, 0);

	/// row-1, col-2
	fMap_ConnIdToPaddle[ std::make_pair(1,113) ] = getPaddleId( fMidTOF,  9, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,114) ] = getPaddleId( fMidTOF, 10, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,115) ] = getPaddleId( fMidTOF, 11, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,116) ] = getPaddleId( fMidTOF, 12, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,117) ] = getPaddleId( fMidTOF, 13, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,118) ] = getPaddleId( fMidTOF, 14, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,119) ] = getPaddleId( fMidTOF, 15, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,120) ] = getPaddleId( fMidTOF, 16, 0);

	/// row-2, col-2
	fMap_ConnIdToPaddle[ std::make_pair(1,121) ] = getPaddleId( fMPD, 1, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,122) ] = getPaddleId( fMPD, 1, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,123) ] = getPaddleId( fMPD, 1, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,124) ] = getPaddleId( fMPD, 2, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,125) ] = getPaddleId( fMPD, 2, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,126) ] = getPaddleId( fMPD, 2, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,127) ] = getPaddleId( fMPD, 3, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,128) ] = getPaddleId( fMPD, 3, 1);

	/// row-3, col-2
	fMap_ConnIdToPaddle[ std::make_pair(1,96) ] = getPaddleId( fMPD, 3, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,95) ] = getPaddleId( fMPD, 4, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,94) ] = getPaddleId( fMPD, 4, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,93) ] = getPaddleId( fMPD, 4, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,92) ] = getPaddleId( fMPD, 5, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,91) ] = getPaddleId( fMPD, 5, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,90) ] = getPaddleId( fMPD, 5, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,89) ] = getPaddleId( fMPD, 6, 0);

	/// row-4, col-2
	fMap_ConnIdToPaddle[ std::make_pair(1,81) ] = getPaddleId( fMPD, 6, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,82) ] = getPaddleId( fMPD, 6, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,83) ] = getPaddleId( fMPD, 7, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,84) ] = getPaddleId( fMPD, 7, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,85) ] = getPaddleId( fMPD, 7, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,86) ] = getPaddleId( fMPD, 8, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,87) ] = getPaddleId( fMPD, 8, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,88) ] = getPaddleId( fMPD, 8, 2);
	
	//////////////
	/// ASIC-0 ///
	//////////////

	/// row-1, col-1
	fMap_ConnIdToPaddle[ std::make_pair(1,24) ] = getPaddleId( fMPD,  9, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,23) ] = getPaddleId( fMPD,  9, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,22) ] = getPaddleId( fMPD,  9, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,21) ] = getPaddleId( fMPD, 10, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,20) ] = getPaddleId( fMPD, 10, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,19) ] = getPaddleId( fMPD, 10, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,18) ] = getPaddleId( fMPD, 11, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,17) ] = getPaddleId( fMPD, 11, 1);

	/// row-2, col-1
	fMap_ConnIdToPaddle[ std::make_pair(1,25) ] = getPaddleId( fMPD, 11, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,26) ] = getPaddleId( fMPD, 12, 0); 
	fMap_ConnIdToPaddle[ std::make_pair(1,27) ] = getPaddleId( fMPD, 12, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,28) ] = getPaddleId( fMPD, 12, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,29) ] = getPaddleId( fMPD, 13, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,30) ] = getPaddleId( fMPD, 13, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,31) ] = getPaddleId( fMPD, 13, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,32) ] = getPaddleId( fMPD, 14, 0);

	/// row-3, col-1
	fMap_ConnIdToPaddle[ std::make_pair(1,64) ] = getPaddleId( fMPD, 14, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,63) ] = getPaddleId( fMPD, 14, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,62) ] = getPaddleId( fMPD, 15, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,61) ] = getPaddleId( fMPD, 15, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,60) ] = getPaddleId( fMPD, 15, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,59) ] = getPaddleId( fMPD, 16, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,58) ] = getPaddleId( fMPD, 16, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,57) ] = getPaddleId( fMPD, 16, 2);

	/// row-4, col-1
	fMap_ConnIdToPaddle[ std::make_pair(1,56) ] = getPaddleId( fMPD, 17, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,55) ] = getPaddleId( fMPD, 17, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,54) ] = getPaddleId( fMPD, 17, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,53) ] = getPaddleId( fMPD, 18, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,52) ] = getPaddleId( fMPD, 18, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,51) ] = getPaddleId( fMPD, 18, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,50) ] = getPaddleId( fMPD, 19, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,49) ] = getPaddleId( fMPD, 19, 1);

	/// row-1, col-2
	fMap_ConnIdToPaddle[ std::make_pair(1,16) ] = getPaddleId( fMPD, 19, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,15) ] = getPaddleId( fMPD, 20, 0);
	fMap_ConnIdToPaddle[ std::make_pair(1,14) ] = getPaddleId( fMPD, 20, 1);
	fMap_ConnIdToPaddle[ std::make_pair(1,13) ] = getPaddleId( fMPD, 20, 2);
	fMap_ConnIdToPaddle[ std::make_pair(1,12) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,11) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,10) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1, 9) ] = 0xFFFF;

	/// row-2, col-2
	fMap_ConnIdToPaddle[ std::make_pair(1,1) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,2) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,3) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,4) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,5) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,6) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,7) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,8) ] = 0xFFFF;

	/// row-3, col-2
	fMap_ConnIdToPaddle[ std::make_pair(1,40) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,39) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,38) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,37) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,36) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,35) ] = getPaddleId( fPPS, 0, 0 );
	fMap_ConnIdToPaddle[ std::make_pair(1,34) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,33) ] = 0xFFFF;

	/// row-3, col-2
	fMap_ConnIdToPaddle[ std::make_pair(1,48) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,47) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,46) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,45) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,44) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,43) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,42) ] = 0xFFFF;
	fMap_ConnIdToPaddle[ std::make_pair(1,41) ] = 0xFFFF;

	return;
}

void TOF_PaddleChannelMap::fillMapPaddleToConnId()
{

	if( fMap_ConnIdToPaddle.empty() ) {
		std::cerr << "[ERR] Empty Map from Paddle to Connector IDs of FEB-S" << std::endl;
		return;
	}

	for( int febS=0; febS<2; febS++ )
	{
		for( int i=0; i<128; i++ )
		{
			uint8_t connID = i+1;
			auto pair =  std::make_pair(febS, connID);
			uint16_t paddle = fMap_ConnIdToPaddle[ pair ];
			fMap_PaddleToConnId[ paddle ] = pair;
		}
	}

	return;
}

void TOF_PaddleChannelMap::fillMapChannelIdPaddle()
{
	auto theChanConv = TOF_ChannelConversion::getInstance();
	auto activeConn_FebD = theChanConv->getActiveConnIdOnFebD();

	for( int d=0; d<2; d++ )
	{
		for( int i=0; i<128; i++ )
	  {
	  	uint8_t connID_FebS = i+1;
	    auto chanID = theChanConv->getAbsoluteChannelID( activeConn_FebD[d], connID_FebS );
	  	auto paddle = fMap_ConnIdToPaddle[ std::make_pair(activeConn_FebD[d], connID_FebS)];

			fMap_PaddleToChannelId[ paddle ] = chanID;
			fMap_ChannelIdToPaddle[ chanID ] = paddle;
		}
	}

	return;
}
