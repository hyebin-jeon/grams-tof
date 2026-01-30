#pragma once

#include "TObject.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TTimeStamp.h"
#include "TOF_TreeDataStg1.h"
#include "TOF_TreeDataStg2.h"
#include "TOF_Constants.h"
#include <iostream>

#ifndef _TOF_CONVERTSTG1TOSTG2_H
#define _TOF_CONVERTSTG1TOSTG2_H

class TOF_ConvertStg1toStg2 : public TObject
{
	public: 
	  //TOF_ConvertStg1toStg2() = default;
	  TOF_ConvertStg1toStg2() {
			setClassStg1();
			setClassStg2();
			return;
		}
    
		inline static TOF_ConvertStg1toStg2 * theConvStg{nullptr}; 
		static TOF_ConvertStg1toStg2 *getInstance() {
			if( theConvStg == nullptr ) {
				theConvStg = new TOF_ConvertStg1toStg2;
			}
			return theConvStg;
		}

    ~TOF_ConvertStg1toStg2() = default;

	private:
		TOF_TreeDataStg1* fStg1{nullptr}; // input
		TOF_TreeDataStg2* fStg2{nullptr}; // output
		void setClassStg1();
		void setClassStg2();

	public: 
		int  setInputPathStg1( const char* fpath );
		int  addTimestampBranches();
		void convertStg1ToStg2( const char* kPathStg1, const char* kPathStg2="" );
		TOF_TreeDataStg1* getStg1() {return fStg1; };
		TOF_TreeDataStg2* getStg2() {return fStg2; };

	
		ClassDef(TOF_ConvertStg1toStg2, 1)
};

#endif
