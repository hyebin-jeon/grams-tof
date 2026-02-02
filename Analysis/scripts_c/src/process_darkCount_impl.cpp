#include "process_darkCount.h"

#include <TOF_ChannelConversion.h>
#include <TFile.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TLine.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

bool runProcessDarkCount(const std::string& inputFile, 
                  const std::string& outputBase,
									const std::string& discFile )
{

	bool drawDisc = false;
	if( !discFile.empty() ) drawDisc = true;

	
  std::ifstream fin( inputFile.c_str() );
	if( ! fin.is_open() ) {
		cerr << "[ERR] Input File Does NOT Exist." << endl;
		return 0;
	}
	else {
		cout << "[INFO] Input File: " << inputFile << endl;
		if( !((TString) inputFile).EndsWith("dark.tsv") )
		{
			cout << "[WARN] Input File Does Not End with \'dark.tsv\'" << endl;
		}
	}

  auto theChanConv = TOF_ChannelConversion::getInstance();

	/// output naming
	const char* inputFile_c = inputFile.c_str();
	TString name_tsv = std::filesystem::path(inputFile_c).filename().c_str();
	TString name_file = (TString) name_tsv(0, name_tsv.Index( ".tsv" ));
	TString name_dir  = std::filesystem::path(inputFile_c).parent_path().c_str();
	TString name_output;
	if( outputBase.empty() ) name_output = Form("%s/%s.root", name_dir.Data()   , name_file.Data());
	else                     name_output = Form("%s/%s.root", outputBase.c_str(), name_file.Data());

	TFile* fout = new TFile( name_output.Data(), "recreate" );

	std::vector< uint32_t > vChannel;
	std::map< uint32_t, TProfile* > vProf_t1;
	std::map< uint32_t, TProfile* > vProf_t2;
	std::map< uint32_t, TProfile* > vProf_e ;


	/// read input file ///
  int portID, slaveID, chipID, channelID;
	std::string vthName;
	double threshold, darkCount;
  std::string sWord, sLine;
  std::stringstream ssLine;
  int lineN{0}, wordN{0};
  const int line0 = 0; // first line Nb
  const char fSep= '\t';

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
			else if( wordN==4  ) vthName   = sWord;
			else if( wordN==5  ) threshold = std::atof(sWord.c_str());
			else if( wordN==6  ) darkCount = std::atof(sWord.c_str());
      else cout << "[WARNING] Too many words :(" << endl;

      wordN++;
    }
	
		auto achanID = theChanConv->getAbsoluteChannelID( portID, slaveID, chipID, channelID );
		//auto achanID = 131072*portID + 4096*slaveID + 64*chipID + channelID;
		//cout << Form("[%03d] %2d, %2d, %2d, %03d, %6s, %2.0f, %6.5f", lineN, portID, slaveID, chipID, channelID, vthName.c_str(), threshold, darkCount ) << endl;

		if( vChannel.size()==0 ) vChannel.push_back( achanID );
		else {
			bool chOK = true;
			for( auto ch: vChannel )
			{
				if( ch == achanID ) chOK = false;
			}
			if( chOK==true ) vChannel.push_back( achanID );
		}

		if( vthName == "vth_t1" ) {
		  if( !vProf_t1[achanID] ) {
				vProf_t1[achanID] = new TProfile( Form("pf_ch%03d_d%d_J%03d_%s", achanID, theChanConv->getConnIdOnFebD(achanID), theChanConv->getConnIdOnFebS(achanID), vthName.c_str()), "", 64, 0, 64 );
			}

			vProf_t1[achanID]->Fill( threshold, darkCount );
		}
		else if( vthName == "vth_t2" ) {
		  if( !vProf_t2[achanID] ) {
				vProf_t2[achanID] = new TProfile( Form("pf_ch%03d_d%d_J%03d_%s", achanID, theChanConv->getConnIdOnFebD(achanID), theChanConv->getConnIdOnFebS(achanID), vthName.c_str()), "", 64, 0, 64 );
			}

			vProf_t2[achanID]->Fill( threshold, darkCount );
		}
		else if( vthName == "vth_e" ) {
		  if( !vProf_e[achanID] ) {
				vProf_e[achanID] = new TProfile( Form("pf_ch%03d_d%d_J%03d_%s", achanID, theChanConv->getConnIdOnFebD(achanID), theChanConv->getConnIdOnFebS(achanID), vthName.c_str()), "", 64, 0, 64 );
			}

			vProf_e[achanID]->Fill( threshold, darkCount );
		}
		else {
			cout << "[WARN] Threshold Name is Out Of Range." << endl;
		}

		lineN++;
	}

	/// channel list
	int chanN = 0;
	cout << "Channel List: " << endl;
	for( auto achanID : vChannel )
	{
		cout << Form("%03d, ", achanID);
		if( chanN%20 == 0 ) cout << endl;
		if( chanN+1 == vChannel.size() ) cout << endl;
		chanN++;
	}


	fout->cd();
	auto dir_t1 = fout->mkdir( "vth_t1" );
	auto dir_t2 = fout->mkdir( "vth_t2" );
	auto dir_e  = fout->mkdir( "vth_e" );

	for( auto achanID : vChannel )
	{
		dir_t1->cd();
		if( vProf_t1[achanID] ) vProf_t1[achanID]->Write();
		dir_t2->cd();
		if( vProf_t2[achanID] ) vProf_t2[achanID]->Write();
		dir_e->cd();
		if( vProf_e[achanID]  ) vProf_e[achanID] ->Write();
	}

	if( drawDisc == false ) {
		fout->Close();
	  cout << "[INFO] Generated Output File: " << fout->GetName() << endl;
		return 1;
	}



	///-----------------------------------------------///
	/// draw threshold lines 
	///-----------------------------------------------///
	
  
	std::ifstream fin_disc( discFile.c_str() );
	if( ! fin_disc.is_open() ) {
		cerr << "[ERR] Input Discriminator Calibration File Does NOT Exist." << endl;
		return 0;
	}
	else {
		cout << "[INFO] Input Discriminator File: " << discFile << endl;
	}

	/// read disc_calibration.tsv
	int baseT, baseE;
	double zeroT1, zeroT2, zeroE;
	double noiseT1, noiseT2, noiseE;
	double vth_t1, vth_t2, vth_e;
  lineN=0; wordN=0;
	std::map< uint32_t, std::array<double,2>> vThreshold_t1;

  while( std::getline(fin_disc, sLine) ) {
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

		auto achanID = theChanConv->getAbsoluteChannelID( portID, slaveID, chipID, channelID );
		vThreshold_t1[achanID] = { zeroT1, noiseT1 };

		//vth_t1 = zeroT1 - noiseT1;
		//vth_t2 = zeroT2 - noiseT2;
		//vth_e  = zeroE  - noiseE;
	}

	/// draw
	auto dir_t1_canv = fout->mkdir( "vth_t1_canv" );
	dir_t1_canv->cd();

	TCanvas* c1 = new TCanvas("c1", "c1");
	TLine* line = new TLine();
	line->SetLineColor(kRed);
	line->SetLineStyle(2);
	for( auto achanID : vChannel )
	{
		c1->SetName( Form("c_%s", vProf_t1[achanID]->GetName()) );
		vProf_t1[achanID]->Draw();
		vProf_t1[achanID]->SetMinimum(0.1);
		gPad->SetLogy();
		gPad->Modified();
		gPad->Update();
		//auto ymax  = gPad->GetUymax();
		auto ymax1 = vProf_t1[achanID]->GetMaximum() * 1.15;

		auto zero  = vThreshold_t1[achanID][0];
		auto noise = vThreshold_t1[achanID][1];
		line->DrawLine( zero      , 0.1, zero      , ymax1 );
		line->DrawLine( zero-noise, 0.1, zero-noise, ymax1 );

		c1->Write();
	}

	fout->Close();
	cout << "[INFO] Generated Output File: " << fout->GetName() << endl;

	return 1;
}

