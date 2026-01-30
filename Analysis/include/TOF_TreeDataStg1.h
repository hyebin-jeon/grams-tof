#pragma once

#ifndef _TOF_TREEDATASTG1_H
#define _TOF_TREEDATASTG1_H

#include "TFile.h"
#include "TTree.h"
#include "TObject.h"
#include "TTimeStamp.h"
#include "TOF_Constants.h"
#include <iostream>
#include <filesystem>

class TOF_TreeDataStg1 : public TObject
{
	public: 
	  TOF_TreeDataStg1() = default;

		explicit TOF_TreeDataStg1(bool init) { 
        if (init) setBranchAddress();
    }

    virtual ~TOF_TreeDataStg1() = default;

	public:
		virtual int getStgNb() const {return 1; }

	public: 
		/// read
		int  isFileStg( const char* fpath );
		int  setInputPath( const char* fpath ); // set fTFile and fTTree
		void setBranchStatus( const char* bname, bool status );
		virtual void setBranchAddress();

		/// write
		void setOutputPath( const char* fpath, const char* opt="recreate" );
		virtual void makeBranches();

		void closeTFile(){ fTFile->Close(); };

	protected:
		bool        fFileExist{false};
		std::string fFilePath;
		std::string fFileName;

		TFile* fTFile{nullptr}; //  TFile needs to be open while using TTree
		TTree* fTTree{nullptr};

	public:
		std::string getFilePath();
		std::string getFileName();
		TTimeStamp  getTimestampCPU();

		TTree* getTTree() { return fTTree; };
		TFile* getTFile() { return fTFile; };
	
	protected: 
    float           step1;
    float           step2;
    long long       stepBegin;
    long long       stepEnd;
    long long       frameID;
    unsigned int    channelID;
    unsigned short  tacID;
    unsigned short  tCoarse;
    unsigned short  eCoarse;
    unsigned short  tFine;
    unsigned short  eFine;

	public:
    float		   getStep1()    { return step1;     };
    float		   getStep2()    { return step2;     };
    uint64_t   getStepBgin() { return stepBegin; };
    uint64_t   getStepEnd()  { return stepEnd;   };
    uint64_t   getFrameID()  { return frameID;   };
    uint32_t   getChannelID(){ return channelID; };
    uint8_t    getTacID()    { return tacID;     };
    uint16_t   getTCoarse()  { return tCoarse;   };
    uint16_t   getECoarse()  { return eCoarse;   };
    uint16_t   getTFine()    { return tFine;     };
    uint16_t   getEFine()    { return eFine;     };

    void setStep1    (float    val){ step1     = val; };
    void setStep2    (float    val){ step2     = val; };
    void setStepBgin (uint64_t val){ stepBegin = val; };
    void setStepEnd  (uint64_t val){ stepEnd   = val; };
    void setFrameID  (uint64_t val){ frameID   = val; };
    void setChannelID(uint32_t val){ channelID = val; };
    void setTacID    (uint8_t  val){ tacID     = val; };
    void setTCoarse  (uint16_t val){ tCoarse   = val; };
    void setECoarse  (uint16_t val){ eCoarse   = val; };
    void setTFine    (uint16_t val){ tFine     = val; };
    void setEFine    (uint16_t val){ eFine     = val; };

	public:
		Long64_t getEntries() { return fTTree->GetEntries(); };
		Int_t 	 getEntry( Long64_t entry, Int_t getall=0) { return fTTree->GetEntry( entry ); };
		void             resetBranchAddresses() { fTTree->ResetBranchAddresses(); };
		Int_t            fillTTree() { return fTTree->Fill(); };


	ClassDef(TOF_TreeDataStg1, 1)

};

#endif
