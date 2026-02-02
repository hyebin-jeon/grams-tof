#pragma once

#ifndef _TOF_TREEDATASTG2_H
#define _TOF_TREEDATASTG2_H

//#include "TFile.h"
//#include "TTree.h"
//#include "TObject.h"
//#include "TTimeStamp.h"
//#include "TOF_Constants.h"
#include "TOF_TreeDataStg1.h"
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
	
	public:
    TTimeStamp getTimeStampCPU() const { return *ts_cpu; };
    TTimeStamp getTimeStampPPS() const { return *ts_pps; };

    void setTimeStampCPU(const TTimeStamp* ts){ if( ts ) *ts_cpu = *ts; };
    void setTimeStampPPS(const TTimeStamp* ts){ if( ts ) *ts_pps = *ts; };

	ClassDefOverride(TOF_TreeDataStg2, 1)

};

#endif
