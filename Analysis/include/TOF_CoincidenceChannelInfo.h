#pragma once

#include "TObject.h"
#include "TTimeStamp.h"

class TOF_CoincidenceChannelInfo : public TObject
{
	public:
		TOF_CoincidenceChannelInfo();
		~TOF_CoincidenceChannelInfo();
	public:
	TTimeStamp      ts_cpu;
	TTimeStamp      ts_pps;
	long long       frameID;
  //unsigned int    channelID;
  uint32_t        channelID;
  unsigned short  tacID;
  unsigned short  tCoarse;
  unsigned short  eCoarse;
  unsigned short  tFine;
  unsigned short  eFine;
	double          timeBegin;
	double          timeEnd;
	double          qdc_cal;
	
	ClassDef(TOF_CoincidenceChannelInfo, 1)
}; 
