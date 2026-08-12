#include "TOF_ActiveAsicList.h"

ClassImp( TOF_ActiveAsicList );

std::string TOF_ActiveAsicList::getDefaultInputFile() 
{
	std::string defaultInputFile="";
		
	const char* glib = std::getenv("GLIB");
  if (glib) {
      defaultInputFile = std::string(glib) + "/config/active_asic_list.tsv";
  }
	else {
	  std::cout << "[WARN] TOF_ActiveAsicList -> $GLIB is not found." << std::endl;
		defaultInputFile = "active_asic_list.tsv";
	}
	  
	std::cout << "[INFO] TOF_ActiveAsicList -> active_asic_list path: " << defaultInputFile << std::endl;
		
	return defaultInputFile;
}

void TOF_ActiveAsicList::useDefaultInputFile() 
{
	fInputFile = getDefaultInputFile();
}

		
void TOF_ActiveAsicList::setInputFile( std::string fname ) 
{ 
	std::cout << "TOF_ActiveAsicList::setInputFile ==> input filename: " << fname << std::endl;
	if( fInputFile.empty() ) fInputFile = getDefaultInputFile();
	else fInputFile = fname; 

	return;
};

int TOF_ActiveAsicList::readActiveAsicList()
{
	std::cout << "TOF_ActiveAsicList::readActiveAsicList()" << std::endl;

  std::ifstream fin( fInputFile.c_str() );
	if( ! fin.is_open() ) {
		std::cout<< Form( "[ERR] Input File Does NOT Exist: %s", fInputFile.c_str() ) << std::endl;
		return TOF_ERR;
	}

  std::string word, sLine;
  std::stringstream ssLine;
	const int line0 = 1;
  int wordN{0}, lineN{0};
	int portID, slaveID;
	std::vector<int> vFebD_connID;

	/// read line by line
  while( std::getline(fin, sLine) )
  {
    ssLine.clear();
    ssLine << sLine;

		/// break a line to words
		/// the scan param table should use '\t' to separate variables
    wordN=0;
    while( std::getline(ssLine, word, '\t') ) 
    {   
		  if( lineN< line0 ) {lineN++; continue;}

			if     ( wordN==0 ) portID  = std::atoi( word.c_str() );
			else if( wordN==1 ) slaveID = std::atoi( word.c_str() );
			else if( wordN==2 ) vFebD_connID.push_back( std::atoi( word.c_str() ) );
			else std::cout << "[Warning] Too Many Scan Parameter values.." << std::endl;

			wordN++;
		}

		lineN++;
	}

	if( vFebD_connID.size()!=2 ) return TOF_ERR;

	fFebD_connID[0] = vFebD_connID.at(0);
	fFebD_connID[1] = vFebD_connID.at(1);

	return TOF_GOOD;
}
		
void TOF_ActiveAsicList::setActiveConnIdOnFebD( uint8_t febD_connID_a, uint8_t febD_connID_b )
{
	fFebD_connID[0] = febD_connID_a;
	fFebD_connID[1] = febD_connID_b;

	return;
}

