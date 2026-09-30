#pragma once

#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <array>
#include "TObject.h"
#include "TString.h"
#include "TOF_Constants.h"

class TOF_ActiveAsicList : public TObject
{
  public:
    inline static TOF_ActiveAsicList * theAsicList{nullptr}; 
  	static TOF_ActiveAsicList *getInstance() {
  		if( theAsicList == nullptr ) {
  			theAsicList = new TOF_ActiveAsicList;
  		}
  		return theAsicList;
  	};
  
  	TOF_ActiveAsicList() 
			: fInputFile( getDefaultInputFile() ),
			  //fFebD_connID{0xFF, 0xFF}
			  fFebD_connID{5, 0xFF}
		{}

  	virtual ~TOF_ActiveAsicList() = default;
		
	private: 
		uint8_t fFebD_connID[2];
		std::string fInputFile;

	public:
		std::string getDefaultInputFile();
		void useDefaultInputFile();
		void setInputFile( std::string fname );
		int  readActiveAsicList();
		//int  readActiveAsicList( const std::string fname );
		void setActiveConnIdOnFebD( uint8_t febD_connID_a, uint8_t febD_connID_b );
		std::array<uint8_t,2> getActiveConnIdOnFebD() { return {fFebD_connID[0], fFebD_connID[1]}; };
		
	  ClassDef(TOF_ActiveAsicList, 1)

};
