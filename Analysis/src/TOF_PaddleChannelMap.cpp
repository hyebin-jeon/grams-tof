#include "TOF_PaddleChannelMap.h"

ClassImp( TOF_PaddleChannelMap );

uint16_t TOF_PaddleChannelMap::getPaddleIdx( uint16_t systemIdx, uint16_t paddleLocIdx, uint16_t sipmLocIdx )
{
	/// systemIdx = UTOF, MTOF, MPD
	/// paddleIdx = [1,16] 
	/// channelIdx = Bias side (0), DAQ side (1)
	uint16_t rval = (systemIdx<< 4*3) + (paddleLocIdx<<4) + sipmLocIdx;
	//std::cout << Form("System: %d, Paddle LocID: %2d, Channel locID: %d --> Paddle Idx: 0x%04X", systemIdx, paddleLocIdx, sipmLocIdx, rval) << std::endl;

	return rval;
}
		
void TOF_PaddleChannelMap::fillMapConnIdToPaddleId()
{
	//////////////////////////////////////////
	/// FEB-S #0
	//////////////////////////////////////////

	////////////////////////////
	/// ASIC-1 --> DAQ Side  ///
	////////////////////////////

  /// row-3, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,65) ] = getPaddleIdx( eSystem::fUTOF, 1, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,66) ] = getPaddleIdx( eSystem::fUTOF, 2, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,67) ] = getPaddleIdx( eSystem::fUTOF, 3, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,68) ] = getPaddleIdx( eSystem::fUTOF, 4, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,69) ] = getPaddleIdx( eSystem::fUTOF, 5, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,70) ] = getPaddleIdx( eSystem::fUTOF, 6, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,71) ] = getPaddleIdx( eSystem::fUTOF, 7, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,72) ] = getPaddleIdx( eSystem::fUTOF, 8, eCoordinate::fDaqSide);

  /// row-4, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,73) ] = getPaddleIdx( eSystem::fUTOF,  9, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,74) ] = getPaddleIdx( eSystem::fUTOF, 10, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,75) ] = getPaddleIdx( eSystem::fUTOF, 11, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,76) ] = getPaddleIdx( eSystem::fUTOF, 12, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,77) ] = getPaddleIdx( eSystem::fUTOF, 13, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,78) ] = getPaddleIdx( eSystem::fUTOF, 14, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,79) ] = getPaddleIdx( eSystem::fUTOF, 15, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,80) ] = getPaddleIdx( eSystem::fUTOF, 16, eCoordinate::fDaqSide);

	/// row-4, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,81) ] = getPaddleIdx( eSystem::fUTOF, 17, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,82) ] = getPaddleIdx( eSystem::fUTOF, 18, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,83) ] = getPaddleIdx( eSystem::fUTOF, 19, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,84) ] = getPaddleIdx( eSystem::fUTOF, 20, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,85) ] = getPaddleIdx( eSystem::fUTOF, 21, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,86) ] = getPaddleIdx( eSystem::fUTOF, 22, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,87) ] = getPaddleIdx( eSystem::fUTOF, 23, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,88) ] = getPaddleIdx( eSystem::fUTOF, 24, eCoordinate::fDaqSide);

	/// row-3, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,89) ] = getPaddleIdx( eSystem::fPPS, 0, eCoordinate::fDummy );
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,90) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,91) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,92) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,93) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,94) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,95) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,96) ] = 0;

  /// row-2, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 97) ] = getPaddleIdx( eSystem::fTest, 0, eCoordinate::fAsicSide ); // spare paddle placed on UTOF (=0)
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 98) ] = getPaddleIdx( eSystem::fTest, 1, eCoordinate::fAsicSide ); // spare paddle placed on MTOF (=1)
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 99) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,100) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,101) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,102) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,103) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,104) ] = 0;

	/// row-1, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,105) ] = getPaddleIdx( eSystem::fMTOF, 1, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,106) ] = getPaddleIdx( eSystem::fMTOF, 2, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,107) ] = getPaddleIdx( eSystem::fMTOF, 3, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,108) ] = getPaddleIdx( eSystem::fMTOF, 4, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,109) ] = getPaddleIdx( eSystem::fMTOF, 5, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,110) ] = getPaddleIdx( eSystem::fMTOF, 6, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,111) ] = getPaddleIdx( eSystem::fMTOF, 7, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,112) ] = getPaddleIdx( eSystem::fMTOF, 8, eCoordinate::fDaqSide);

	/// row-1, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,113) ] = getPaddleIdx( eSystem::fMTOF,  9, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,114) ] = getPaddleIdx( eSystem::fMTOF, 10, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,115) ] = getPaddleIdx( eSystem::fMTOF, 11, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,116) ] = getPaddleIdx( eSystem::fMTOF, 12, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,117) ] = getPaddleIdx( eSystem::fMTOF, 13, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,118) ] = getPaddleIdx( eSystem::fMTOF, 14, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,119) ] = getPaddleIdx( eSystem::fMTOF, 15, eCoordinate::fDaqSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,120) ] = getPaddleIdx( eSystem::fMTOF, 16, eCoordinate::fDaqSide);

	/// row-2, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,121) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,122) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,123) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,124) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,125) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,126) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,127) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,128) ] = 0;

	
	////////////////////////////
	/// ASIC-0 --> Bias Side ///
	////////////////////////////
	
	/// row-2, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 1) ] = getPaddleIdx( eSystem::fUTOF,  1, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 2) ] = getPaddleIdx( eSystem::fUTOF,  2, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 3) ] = getPaddleIdx( eSystem::fUTOF,  3, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 4) ] = getPaddleIdx( eSystem::fUTOF,  4, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 5) ] = getPaddleIdx( eSystem::fUTOF,  5, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 6) ] = getPaddleIdx( eSystem::fUTOF,  6, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 7) ] = getPaddleIdx( eSystem::fUTOF,  7, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 8) ] = getPaddleIdx( eSystem::fUTOF,  8, eCoordinate::fBiasSide);

	/// row-1, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(0, 9) ] = getPaddleIdx( eSystem::fUTOF,  9, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,10) ] = getPaddleIdx( eSystem::fUTOF, 10, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,11) ] = getPaddleIdx( eSystem::fUTOF, 11, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,12) ] = getPaddleIdx( eSystem::fUTOF, 12, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,13) ] = getPaddleIdx( eSystem::fUTOF, 13, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,14) ] = getPaddleIdx( eSystem::fUTOF, 14, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,15) ] = getPaddleIdx( eSystem::fUTOF, 15, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,16) ] = getPaddleIdx( eSystem::fUTOF, 16, eCoordinate::fBiasSide);


	/// row-1, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,17) ] = getPaddleIdx( eSystem::fUTOF, 17, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,18) ] = getPaddleIdx( eSystem::fUTOF, 18, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,19) ] = getPaddleIdx( eSystem::fUTOF, 19, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,20) ] = getPaddleIdx( eSystem::fUTOF, 20, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,21) ] = getPaddleIdx( eSystem::fUTOF, 21, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,22) ] = getPaddleIdx( eSystem::fUTOF, 22, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,23) ] = getPaddleIdx( eSystem::fUTOF, 23, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,24) ] = getPaddleIdx( eSystem::fUTOF, 24, eCoordinate::fBiasSide);

	/// row-2, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,25) ] = getPaddleIdx( eSystem::fTrig, 0, eCoordinate::fDummy );
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,26) ] = 0; 
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,27) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,28) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,29) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,30) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,31) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,32) ] = 0;

	/// row-3, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,33) ] = getPaddleIdx( eSystem::fTest, 0, eCoordinate::fCleanSide ); // spare paddle placed on UTOF (=0)
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,34) ] = getPaddleIdx( eSystem::fTest, 1, eCoordinate::fCleanSide ); // spare paddle placed on MTOF (=1)
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,35) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,36) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,37) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,38) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,39) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,40) ] = 0;

	/// row-3, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,41) ] = getPaddleIdx( eSystem::fMTOF,  1, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,42) ] = getPaddleIdx( eSystem::fMTOF,  2, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,43) ] = getPaddleIdx( eSystem::fMTOF,  3, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,44) ] = getPaddleIdx( eSystem::fMTOF,  4, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,45) ] = getPaddleIdx( eSystem::fMTOF,  5, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,46) ] = getPaddleIdx( eSystem::fMTOF,  6, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,47) ] = getPaddleIdx( eSystem::fMTOF,  7, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,48) ] = getPaddleIdx( eSystem::fMTOF,  8, eCoordinate::fBiasSide);
	
	/// row-4, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,49) ] = getPaddleIdx( eSystem::fMTOF,  9, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,50) ] = getPaddleIdx( eSystem::fMTOF, 10, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,51) ] = getPaddleIdx( eSystem::fMTOF, 11, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,52) ] = getPaddleIdx( eSystem::fMTOF, 12, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,53) ] = getPaddleIdx( eSystem::fMTOF, 13, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,54) ] = getPaddleIdx( eSystem::fMTOF, 14, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,55) ] = getPaddleIdx( eSystem::fMTOF, 15, eCoordinate::fBiasSide);
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,56) ] = getPaddleIdx( eSystem::fMTOF, 16, eCoordinate::fBiasSide);

	/// row-3, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,57) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,58) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,59) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,60) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,61) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,62) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,63) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(0,64) ] = 0;

	/*
	//////////////////////////////////////////
	/// FEB-S #1
	//////////////////////////////////////////
	
	//////////////
	/// ASIC-1 ///
	//////////////

	/// row-1, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,105) ] = getPaddleIdx( fUTOF, 1, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,106) ] = getPaddleIdx( fUTOF, 2, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,107) ] = getPaddleIdx( fUTOF, 3, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,108) ] = getPaddleIdx( fUTOF, 4, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,109) ] = getPaddleIdx( fUTOF, 5, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,110) ] = getPaddleIdx( fUTOF, 6, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,111) ] = getPaddleIdx( fUTOF, 7, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,112) ] = getPaddleIdx( fUTOF, 8, 1);

  /// row-2, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(1, 97) ] = getPaddleIdx( fUTOF,  9, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1, 98) ] = getPaddleIdx( fUTOF, 10, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1, 99) ] = getPaddleIdx( fUTOF, 11, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,100) ] = getPaddleIdx( fUTOF, 12, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,101) ] = getPaddleIdx( fUTOF, 13, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,102) ] = getPaddleIdx( fUTOF, 14, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,103) ] = getPaddleIdx( fUTOF, 15, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,104) ] = getPaddleIdx( fUTOF, 16, 1);

  /// row-3, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,72) ] = getPaddleIdx( fUTOF, 17, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,71) ] = getPaddleIdx( fUTOF, 18, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,70) ] = getPaddleIdx( fUTOF, 19, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,69) ] = getPaddleIdx( fUTOF, 20, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,68) ] = getPaddleIdx( fUTOF, 21, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,67) ] = getPaddleIdx( fUTOF, 22, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,66) ] = getPaddleIdx( fUTOF, 23, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,65) ] = getPaddleIdx( fUTOF, 24, 1);

  /// row-4, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,73) ] = getPaddleIdx( fMTOF, 1, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,74) ] = getPaddleIdx( fMTOF, 2, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,75) ] = getPaddleIdx( fMTOF, 3, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,76) ] = getPaddleIdx( fMTOF, 4, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,77) ] = getPaddleIdx( fMTOF, 5, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,78) ] = getPaddleIdx( fMTOF, 6, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,79) ] = getPaddleIdx( fMTOF, 7, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,80) ] = getPaddleIdx( fMTOF, 8, 1);

	/// row-1, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,113) ] = getPaddleIdx( fMTOF,  9, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,114) ] = getPaddleIdx( fMTOF, 10, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,115) ] = getPaddleIdx( fMTOF, 11, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,116) ] = getPaddleIdx( fMTOF, 12, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,117) ] = getPaddleIdx( fMTOF, 13, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,118) ] = getPaddleIdx( fMTOF, 14, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,119) ] = getPaddleIdx( fMTOF, 15, 1);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,120) ] = getPaddleIdx( fMTOF, 16, 1);

	/// row-2, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,121) ] = getPaddleIdx( fMPD, 1, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,122) ] = getPaddleIdx( fMPD, 1, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,123) ] = getPaddleIdx( fMPD, 1, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,124) ] = getPaddleIdx( fMPD, 2, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,125) ] = getPaddleIdx( fMPD, 2, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,126) ] = getPaddleIdx( fMPD, 2, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,127) ] = getPaddleIdx( fMPD, 3, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,128) ] = getPaddleIdx( fMPD, 3, 4);

	/// row-3, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,96) ] = getPaddleIdx( fMPD, 3, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,95) ] = getPaddleIdx( fMPD, 4, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,94) ] = getPaddleIdx( fMPD, 4, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,93) ] = getPaddleIdx( fMPD, 4, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,92) ] = getPaddleIdx( fMPD, 5, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,91) ] = getPaddleIdx( fMPD, 5, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,90) ] = getPaddleIdx( fMPD, 5, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,89) ] = getPaddleIdx( fMPD, 6, 3);

	/// row-4, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,81) ] = getPaddleIdx( fMPD, 6, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,82) ] = getPaddleIdx( fMPD, 6, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,83) ] = getPaddleIdx( fMPD, 7, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,84) ] = getPaddleIdx( fMPD, 7, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,85) ] = getPaddleIdx( fMPD, 7, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,86) ] = getPaddleIdx( fMPD, 8, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,87) ] = getPaddleIdx( fMPD, 8, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,88) ] = getPaddleIdx( fMPD, 8, 5);
	                                                                    
	//////////////
	/// ASIC-0 ///
	//////////////

	/// row-1, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,24) ] = getPaddleIdx( fMPD,  9, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,23) ] = getPaddleIdx( fMPD,  9, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,22) ] = getPaddleIdx( fMPD,  9, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,21) ] = getPaddleIdx( fMPD, 10, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,20) ] = getPaddleIdx( fMPD, 10, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,19) ] = getPaddleIdx( fMPD, 10, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,18) ] = getPaddleIdx( fMPD, 11, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,17) ] = getPaddleIdx( fMPD, 11, 4);

	/// row-2, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,25) ] = getPaddleIdx( fMPD, 11, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,26) ] = getPaddleIdx( fMPD, 12, 3); 
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,27) ] = getPaddleIdx( fMPD, 12, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,28) ] = getPaddleIdx( fMPD, 12, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,29) ] = getPaddleIdx( fMPD, 13, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,30) ] = getPaddleIdx( fMPD, 13, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,31) ] = getPaddleIdx( fMPD, 13, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,32) ] = getPaddleIdx( fMPD, 14, 3);

	/// row-3, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,64) ] = getPaddleIdx( fMPD, 14, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,63) ] = getPaddleIdx( fMPD, 14, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,62) ] = getPaddleIdx( fMPD, 15, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,61) ] = getPaddleIdx( fMPD, 15, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,60) ] = getPaddleIdx( fMPD, 15, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,59) ] = getPaddleIdx( fMPD, 16, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,58) ] = getPaddleIdx( fMPD, 16, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,57) ] = getPaddleIdx( fMPD, 16, 5);

	/// row-4, col-1
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,56) ] = getPaddleIdx( fMPD, 17, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,55) ] = getPaddleIdx( fMPD, 17, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,54) ] = getPaddleIdx( fMPD, 17, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,53) ] = getPaddleIdx( fMPD, 18, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,52) ] = getPaddleIdx( fMPD, 18, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,51) ] = getPaddleIdx( fMPD, 18, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,50) ] = getPaddleIdx( fMPD, 19, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,49) ] = getPaddleIdx( fMPD, 19, 4);
                                                                        
	/// row-1, col-2                                                      
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,16) ] = getPaddleIdx( fMPD, 19, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,15) ] = getPaddleIdx( fMPD, 20, 3);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,14) ] = getPaddleIdx( fMPD, 20, 4);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,13) ] = getPaddleIdx( fMPD, 20, 5);
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,12) ] = 0;                
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,11) ] = 0;                
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,10) ] = 0;                
	fMap_ConnIdToPaddleIdx[ std::make_pair(1, 9) ] = 0;                
                                                                       
	/// row-2, col-2                                                     
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,1) ] = 0;                 
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,2) ] = 0;                 
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,3) ] = 0;                 
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,4) ] = 0;                 
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,5) ] = 0;                 
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,6) ] = 0;                 
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,7) ] = 0;                 
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,8) ] = 0;                 

	/// row-3, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,40) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,39) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,38) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,37) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,36) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,35) ] = getPaddleIdx( fPPS, 0, 0 );
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,34) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,33) ] = 0;

	/// row-3, col-2
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,48) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,47) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,46) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,45) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,44) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,43) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,42) ] = 0;
	fMap_ConnIdToPaddleIdx[ std::make_pair(1,41) ] = 0;
	*/

	return;
}

void TOF_PaddleChannelMap::fillMapPaddleIdToConnId()
{

	if( fMap_ConnIdToPaddleIdx.empty() ) {
		std::cerr << "[ERR] Empty Map from Paddle to Connector IDs of FEB-S" << std::endl;
		return;
	}

	for( int febS=0; febS<fNbOfFebS; febS++ )
	{
		for( int i=0; i<128; i++ )
		{
			uint8_t connID = i+1;
			auto pair =  std::make_pair(febS, connID);
			uint16_t paddleIdx = fMap_ConnIdToPaddleIdx[ pair ];
			fMap_PaddleIdxToConnIDs[ paddleIdx ] = pair;
		}
	}

	return;
}

int TOF_PaddleChannelMap::getFebSIdx( uint8_t connIdOnFebD )
{
	auto theAsicList = TOF_ActiveAsicList::getInstance();
	auto activeConn_FebD = theAsicList->getActiveConnIdOnFebD();

	int febS_idx;
	if     ( connIdOnFebD == activeConn_FebD[0] ) febS_idx = 0;
	else if( connIdOnFebD == activeConn_FebD[1] ) febS_idx = 1;
	else febS_idx = -1;
	
	if( febS_idx < 0 ) {
		std::cerr << "[ERR] Inconsistent Connector IDs on FEB/D. Check active_asic_list.txt in config directory: " << Form( "input FEB/D connID: %d vs. active connIDs: %d, %d", connIdOnFebD, activeConn_FebD[0], activeConn_FebD[1]) << std::endl;
		return -1;
	}

	return febS_idx;
}

uint16_t TOF_PaddleChannelMap::getPaddleIdx( uint8_t connIdOnFebD, uint8_t connIdOnFebS )
{
	auto febS_idx = getFebSIdx( connIdOnFebD );
	if( febS_idx<0 ) return -1;
	return fMap_ConnIdToPaddleIdx[ std::make_pair( febS_idx, connIdOnFebS ) ];
}

uint8_t TOF_PaddleChannelMap::getSystemIdx( uint8_t connID_D, uint8_t connID_S )
{
	auto paddleIdx = getPaddleIdx( connID_D, connID_S );
	return getSystemIdx( paddleIdx );
}
uint8_t TOF_PaddleChannelMap::getPaddleLocId( uint8_t connID_D, uint8_t connID_S )
{
	auto paddleIdx = getPaddleIdx( connID_D, connID_S );
	return getPaddleLocId( paddleIdx );
}
uint8_t TOF_PaddleChannelMap::getSipmLocId( uint8_t connID_D, uint8_t connID_S )
{
	auto paddleIdx = getPaddleIdx( connID_D, connID_S );
	return getSipmLocId( paddleIdx );
}

bool TOF_PaddleChannelMap::isTriggerChannel( uint16_t paddleIdx )
{
	auto syst = getSystemIdx( paddleIdx );
	bool rval = syst==eSystem::fTrig? true:false;
	return rval;
}
bool TOF_PaddleChannelMap::isPpsChannel( uint16_t paddleIdx )
{
	auto syst = getSystemIdx( paddleIdx );
	bool rval = syst==eSystem::fPPS? true:false;
	return rval;
}
bool TOF_PaddleChannelMap::isTestPaddle( uint16_t paddleIdx )
{
	auto syst = getSystemIdx( paddleIdx );
	bool rval = syst==eSystem::fTest? true:false;
	return rval;
}

void TOF_PaddleChannelMap::dump()
{
	std::cout << "fMap_ConnIdToPaddleIdx.size() = " << fMap_ConnIdToPaddleIdx.size() << std::endl;
	std::cout << "fMap_PaddleIdxToConnIDs.size() = " << fMap_PaddleIdxToConnIDs.size() << std::endl;
	return;
}

