#include "TOF_ParameterScan.h"

ClassImp( TOF_ParameterScan );

void TOF_ParameterScan::readParamScanTable( const char* fname )
{
  std::ifstream fin( fname );
	if( ! fin.is_open() ) {
		std::cout<< Form( "[ERR] Parameter scan file does not exist.Exit(): %s", fname ) << std::endl;
		return;
	}

  std::string word, sLine;
  std::stringstream ssLine;
  int wordN{0}, lineN{0};

	/// read line by line
  while( std::getline(fin, sLine) )
  {
    ssLine.clear();
    ssLine << sLine;

    wordN=0;

		/// break a line to words
		/// the scan param table should use '\t' to separate variables
    while( std::getline(ssLine, word, '\t') ) 
    {   
      TString tWord = word;

			if( lineN==0 ) {
				if     ( wordN==0 ) fParName0 = word;
				else if( wordN==1 ) fParName1 = word;
				else std::cout << "[Warning] Too Many Scan Parameters.." << std::endl;
			}
			else 
			{
				//val[ wordN ] = word.atof();
				//std::cout << tWord << std::endl;
				if     ( wordN==0 ) fParVal0.push_back( tWord.Atof() );
				else if( wordN==1 ) fParVal1.push_back( tWord.Atof() );
				else std::cout << "[Warning] Too Many Scan Parameter values.." << std::endl;
			}

			wordN++;
		}

		lineN++;
	}

	if( fParVal1.size()==0 ) fParName1 = "N/A";

	fNbOfParam0 = fParVal0.size();
	fNbOfParam1 = fParVal1.size();

	//for( auto val: fParVal0 ) std::cout << "val0: " << val << std::endl;
	//for( auto val: fParVal1 ) std::cout << "val1: " << val << std::endl;

	return;
}

void TOF_ParameterScan::printParamScanTable()
{
	std::cout << Form("%s\t%s", fParName0.Data(), fParName1.Data()) << std::endl;

	auto Npar0 = fParVal0.size();
	auto Npar1 = fParVal1.size();

	auto size = Npar0>Npar1? Npar0:Npar1;

	for( int i=0; i<size; i++ )
	{
		if( Npar0 > i && Npar1 > i )
			std::cout << Form("%.1f\t%.1f", fParVal0[i], fParVal1[i]) << std::endl;
		else if( Npar0 > i )
			std::cout << Form("%.1f\tn/a", fParVal0[i]) << std::endl;
		else if( Npar1 > i )
			std::cout << Form("n/a\t%.1f", fParVal1[i]) << std::endl;
		else break;
	}

	return;
}

TString TOF_ParameterScan::getParameterName( int idx )
{
	if( idx==0 ) return fParName0;
	if( idx==1 ) return fParName1;

	return "";
}

std::vector<double> TOF_ParameterScan::getParameterValueList( int idx )
{
	if( idx==0 ) return fParVal0;
	if( idx==1 ) return fParVal1;

	return {};
}

int TOF_ParameterScan::getNbOfParameter( int idx )
{
	if( idx==0 ) return fParVal0.size();
	if( idx==1 ) return fParVal1.size();

	return -1;
}





