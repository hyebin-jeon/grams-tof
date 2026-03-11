#pragma once

#define TOF_GOOD 1
#define TOF_WARNING -1
#define TOF_ERR -2

#define CLOCKS_IN_A_FRAME 1024

#define NUMBER_OF_ASICS   16
#define NUMBER_OF_CHANNELS_OF_ASIC 64
//#define NUMBER_OF_TAC_OF_CHANNEL 4

enum class TOF_Mode {
  fQdc = 0,
  fTot = 1,
};

enum class TOF_Branch {
  fBranchT = 0,
  fBranchE = 1,
};

enum class TOF_Discriminator {
  fDiscT1 = 0,
  fDiscT2 = 2,
  fDiscE  = 3,
};

const double fTOF_TdcClkFreq = 200E6; // 200 MHz
const double fTOF_TdcClkPer = 1./fTOF_TdcClkFreq; // period in sec
const double fTOF_TdcClkPerNs = fTOF_TdcClkPer * 1E9; // period in nsec

/// number of channels, paddles
const int fTOF_NbChannelsPerAsic{ 64};
const int fTOF_NbChannelsPerFebS{128};
const int fTOF_NbChannelsAll    {202}; // 200 sipm channels + 1 pps + 1 tpc trg
const int fTTOF_NbPaddles       { 24};
const int fMTOF_NbPaddles       { 16};
const int fMPD_NbPaddles        { 20};
const int fTTOF_NbChannelsPerEnd {  1};
const int fMTOF_NbChannelsPerEnd {  1};
const int fMPD_NbChannelsPerEnd  {  3};
const int fTTOF_NbChannels      { fTTOF_NbPaddles*fTTOF_NbChannelsPerEnd};
const int fMTOF_NbChannels      { fMTOF_NbPaddles*fMTOF_NbChannelsPerEnd};
const int fMPD_NbChannels       { fMPD_NbPaddles *fMPD_NbChannelsPerEnd };

