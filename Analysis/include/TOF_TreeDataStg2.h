#pragma once

#ifndef _TOF_TREEDATASTG2_H
#define _TOF_TREEDATASTG2_H

#include "TOF_TreeDataStg1.h"
#include "TOF_ChannelConversion.h"
#include <iostream>
#include <filesystem>

class TOF_TreeDataStg2 : public TOF_TreeDataStg1
{
	public: 
	  TOF_TreeDataStg2() : TOF_TreeDataStg1() {
			ts_cpu = new TTimeStamp();
			ts_pps = new TTimeStamp();
		}

		explicit TOF_TreeDataStg2(bool init): TOF_TreeDataStg2() { 
        if (init) setBranchAddress();
    }

    ~TOF_TreeDataStg2() override {
			delete ts_cpu;
			delete ts_pps;
		}

	public:
		int getStgNb() const override {return 2; }
	public: 
		void setBranchAddress() override;
		void makeBranches() override;
	
	private: 
		TTimeStamp* ts_cpu{nullptr};
		TTimeStamp* ts_pps{nullptr};
		uint8_t     connID_febS;
		uint8_t     connID_febD;
		uint16_t    paddleIdx;
		double      tdc_cal; 
		double      qdc_cal; 
	
	public:
    TTimeStamp getTimeStampCPU() const { return *ts_cpu; };
    TTimeStamp getTimeStampPPS() const { return *ts_pps; };
		uint8_t    getConnID_FebD()  const { return connID_febD; };
		uint8_t    getConnID_FebS()  const { return connID_febS; };
		uint16_t   getPaddleIdx()    const { return paddleIdx; }; // 0xABBC A=(0=TTOF, 1=MTOF, 2=MPD, 3=Trig, 4=PPS), BB=paddle ID, C= channel ID within a paddle
		double    getCalibratedTdc() const { return tdc_cal; };
		double    getCalibratedQdc() const { return qdc_cal; };

    void setTimeStampCPU (const TTimeStamp* ts ){ if( ts ) *ts_cpu = *ts; };
    void setTimeStampPPS (const TTimeStamp* ts ){ if( ts ) *ts_pps = *ts; };
		void setConnID_FebD  (const uint8_t  val ){ connID_febD = val; };
		void setConnID_FebS  (const uint8_t  val ){ connID_febS = val; };
		void setPaddleIdx    (const uint16_t val ){ paddleIdx   = val; };
		void setCalibratedTdc(const uint16_t val ){ tdc_cal     = val; };
		void setCalibratedQdc(const uint16_t val ){ qdc_cal     = val; };

	ClassDefOverride(TOF_TreeDataStg2, 1)

};

#endif
