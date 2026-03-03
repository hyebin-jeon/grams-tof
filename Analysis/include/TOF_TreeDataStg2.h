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
		
		/// paddle index
		// [0]: system ID (0 TTOF, 1 MTOF, 2 MPD)
		// [1]: paddle ID 
		// [2]: channel ID
		std::array<uint8_t, 3> paddleIdx;
	
	public:
    TTimeStamp getTimeStampCPU() const { return *ts_cpu; };
    TTimeStamp getTimeStampPPS() const { return *ts_pps; };
		uint8_t    getConnID_FebD()  const { return connID_febD; };
		uint8_t    getConnID_FebS()  const { return connID_febS; };
		uint8_t    getPaddleIdx_System()  const { return paddleIdx[0]; };
		uint8_t    getPaddleIdx_Paddle()  const { return paddleIdx[1]; };
		uint8_t    getPaddleIdx_Channel() const { return paddleIdx[2]; };
		std::array<uint8_t,3> getPaddleIdx() const { return paddleIdx; };

    void setTimeStampCPU(const TTimeStamp* ts ){ if( ts ) *ts_cpu = *ts; };
    void setTimeStampPPS(const TTimeStamp* ts ){ if( ts ) *ts_pps = *ts; };
		void setConnID_FebD (const uint8_t connID ){ connID_febD=connID; };
		void setConnID_FebS (const uint8_t connID ){ connID_febS=connID; };
		void getPaddleIdx_System (const uint8_t id){ paddleIdx[0] = id };
		void getPaddleIdx_Paddle (const uint8_t id){ paddleIdx[1] = id };
		void getPaddleIdx_Channel(const uint8_t id){ paddleIdx[2] = id };
		void getPaddleIdx        (const std::array<uint8_t,3> id){ paddleIdx[0] = id[0]; paddleIdx[1]=id[1]; paddleIdx[2]=id[2]; };

	ClassDefOverride(TOF_TreeDataStg2, 1)

};

#endif
