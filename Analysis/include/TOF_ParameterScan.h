#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "TObject.h"
#include "TString.h"
#include "TOF_Constants.h"

class TOF_ParameterScan: public TObject
{
	public:
    inline static TOF_ParameterScan * theParScan{nullptr}; 
		static TOF_ParameterScan *getInstance() {
			if( theParScan == nullptr ) {
				theParScan = new TOF_ParameterScan;
			}
			return theParScan;
		}

    ~TOF_ParameterScan() = default;

	private:
		int fNbOfParam0;
		int fNbOfParam1;

		TString fParName0;
		TString fParName1;

		std::vector<double> fParVal0;
		std::vector<double> fParVal1;

	public:
		void readParamScanTable( const char* fname );
    void printParamScanTable();

		TString getParameterName( int idx );
		std::vector<double> getParameterValueList( int idx );
		int getNbOfParameter( int idx );
	
		ClassDef(TOF_ParameterScan, 1)
};





