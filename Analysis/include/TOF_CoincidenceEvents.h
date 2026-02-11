/// v2
#pragma once

#ifndef _TOF_COINCIDENCEEVENTS_H
#define _TOF_COINCIDENCEEVENTS_H

#include "TObject.h"
#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TOF_TdcQdcCalibration.h"
#include "TOF_TreeDataStg1.h"
#include "TOF_TreeDataStg2.h"
#include "TOF_CoincidenceChannelInfo.h"
#include "TOF_Fitting.h"
#include "TOF_Attributes.h"

enum class TOF_QdcCalibMethod {
	fGetEnergy = 0,
	fLinear = 1,
	fTiming = 2,
};

class TOF_CoincidenceEvents : public TObject
{
	public:
    inline static TOF_CoincidenceEvents * theCoin{nullptr}; 
  	static TOF_CoincidenceEvents *getInstance() {
  		if( theCoin == nullptr ) {
  			theCoin = new TOF_CoincidenceEvents;
  		}

  		return theCoin;
  	}
  
  	~TOF_CoincidenceEvents();
		
		//TOF_TdcQdcCalibration* theCalib{nullptr};
	
		TOF_TreeDataStg2* fStg2{nullptr};

	private:
		void setClassStg2();
		TTree* fTreeCoin{nullptr};
		std::vector< uint32_t > fActiveChannelList;
	  std::vector<TOF_CoincidenceChannelInfo> vBranch;

	private:
		uint32_t fTrigChannelID{0}; // absolute channel ID of a trigger channelj
		double   fCoinTimeWindow{1};
	  std::map< uint32_t, TOF_CoincidenceChannelInfo> fChannelDataList; // {nullptr};

	public:
		int  setInputPathStg2( const char* fnameStg2 );
		void setTriggerChannel( uint32_t trigCh ) { fTrigChannelID = trigCh; }; // coincidence evt using trigger channel is not prepared yet
		void setActiveChannels( std::vector<uint32_t> chanList );
		void setCoincidenceTimeWindowInClk( double twindow ); // { fCoinTimeWindow = twindow; }; // in clock
		void setCoincidenceTimeWindowInSec( double twindow );
		void reset();

		TTree* getCoincidenceEventsTree();

	/// calibration method
	private:
		TOF_QdcCalibMethod fQdcCalibMethod{ TOF_QdcCalibMethod::fGetEnergy };
	public:
		void setQdcCalibMethod( TOF_QdcCalibMethod calMethod ) { fQdcCalibMethod = calMethod; };
	
	/// for QA
	private:
		const double fTdcFreq  = 200E6; // 200 MHz - confirmed
		const double fTdcClk   = 1./fTdcFreq; // sec
		const double fTdcClkNs = fTdcClk/pow(10,-9); // ns

		TH1F* fHisto_dT{nullptr};        // time resolution (ns)
		TH1F* fHisto_NbOfEvt{nullptr};   // channel vs nb of coincidence events
	  TH2F* fHisto_TvsQcal{nullptr};   // tdiff vs qcal ratio
	public:
		void generateHistoForQA(const char* pdfName);
		TH1F* getHisto_TimeResol()       { return fHisto_dT     ; };
		TH1F* getHisto_ChannelVsNevents(){ return fHisto_NbOfEvt; };
		TH2F* getHisto_TdcVsQcalib()     { return fHisto_TvsQcal; };


	ClassDef(TOF_CoincidenceEvents, 1)

};

#endif

