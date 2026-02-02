#include "process_discSetting.h"

#include <TOF_ChannelConversion.h>
#include <TString.h>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

bool runProcessDiscSetting( const std::string& inputFile, const std::string& outputBase )
{

  std::ifstream fin( inputFile.c_str() );
	if( ! fin.is_open() ) {
		cerr << "[ERR] Input File Does NOT Exist." << endl;
		return 0;
	}
	else {
		cout << "[INFO] Input File: " << inputFile << endl;
	}

	/// output naming
	const char* inputFile_c = inputFile.c_str();
	TString name_dir    = std::filesystem::path(inputFile_c).parent_path().c_str();
	TString name_fin  = std::filesystem::path(inputFile_c).filename().c_str();
	TString name_fout = ((TString) name_fin).ReplaceAll("calibration", "setting");
	TString name_output;
	if( outputBase.empty() ) name_output = Form("%s/%s", name_dir.Data()   , name_fout.Data());
	else                     name_output = Form("%s/%s", outputBase.c_str(), name_fout.Data());

	/// output file
  std::ofstream fout( name_output.Data() );
	if (!fout.is_open()) {
		std::cerr<< "[ERR] Cannot create an output file."<< std::endl;
		return 0;
	}
	else cout << Form("[INFO] Generated Output File: %s", name_output.Data()) << endl;

	/// first line 
	fout << "#portID\tslaveID\tchipID\tchannelID\tvth_t1\tvth_t2\tvth_e\n";


	/// read input file
  int portID, slaveID, chipID, channelID;
	int baseT, baseE;
	double zeroT1, zeroT2, zeroE;
	double noiseT1, noiseT2, noiseE;
	int nline=0;

  std::string sWord, sLine;
  std::stringstream ssLine;
  int lineN{0}, wordN{0};
  int line0 = 1;
  const char fSep= '\t';

	double vth_t1, vth_t2, vth_e;

  while( std::getline(fin, sLine) ) {
    if( lineN < line0 ) {
      lineN++;
      continue;
    }
    ssLine.clear();
    ssLine << sLine;

    wordN = 0;
    while( std::getline(ssLine, sWord, fSep) ) {
      if     ( wordN==0  ) portID    = std::atoi(sWord.c_str());
			else if( wordN==1  ) slaveID   = std::atoi(sWord.c_str());
			else if( wordN==2  ) chipID    = std::atoi(sWord.c_str());
			else if( wordN==3  ) channelID = std::atoi(sWord.c_str());
			else if( wordN==4  ) baseT     = std::atoi(sWord.c_str());
			else if( wordN==5  ) baseE     = std::atoi(sWord.c_str());
			else if( wordN==6  ) zeroT1    = std::atof(sWord.c_str());
			else if( wordN==7  ) zeroT2    = std::atof(sWord.c_str());
			else if( wordN==8  ) zeroE     = std::atof(sWord.c_str());
			else if( wordN==9  ) noiseT1   = std::atof(sWord.c_str());
			else if( wordN==10 ) noiseT2   = std::atof(sWord.c_str());
			else if( wordN==11 ) noiseE    = std::atof(sWord.c_str());
      else cout << "[WARNING] Too many words :(" << endl;

      wordN++;
    }
		
		//cout << Form("[%03d] %2u, %2u, %2u, %03hu, %2d, %2d, %3.4f, %3.4f, %3.4f, %3.4f, %3.4f, %3.4f", lineN, portID, slaveID, chipID, channelID, baseT, baseE, zeroT1, zeroT2, zeroE, noiseT1, noiseT2, noiseE ) << endl;

		/// threshold = mean - 1 sigma (in DAC) - note that higher thresold voltage with smaller DAC
		vth_t1 = zeroT1 - noiseT1;
		vth_t2 = zeroT2 - noiseT2;
		vth_e  = zeroE  - noiseE;

		fout << portID << "\t" << slaveID  << "\t" << chipID  << "\t" << channelID  << "\t" << vth_t1 << "\t" << vth_t2 << "\t" << vth_e << std::endl;

    lineN++;
  }
	
	fout.close();
	fin.close();

	return 1;
}

