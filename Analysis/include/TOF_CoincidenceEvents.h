/// v2

#pragma once

#include "TObject.h"
#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"
#include "TH1D.h"
#include "TH2D.h"
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
  
  	//~TOF_CoincidenceEvents() = default;
  	~TOF_CoincidenceEvents();
		
		//TOF_TdcQdcCalibration* theCalib{nullptr};
	
		TOF_TreeDataStg2* fStg2 = new TOF_TreeDataStg2(); //{nullptr};

	private:
		int fStg2Good{-1};

		//TTree* fTreeStg2{nullptr};
		TTree* fTreeCoin{nullptr};
	  std::vector<TOF_CoincidenceChannelInfo> vBranch;
		//std::vector<std::vector<TOF_CoincidenceChannelInfo>> vTree;

	private:
		bool fUseStg1{false};
		bool fUseStg2{false};
		TOF_TreeDataStg2* fTree{nullptr};
		TOF_TreeDataStg1* fTreeStg1{nullptr};
		uint32_t fTrigChannelID{0}; // absolute channel ID of a trigger channelj
		double   fCoinTimeWindow{1};
		TOF_QdcCalibMethod fQdcCalibMethod{ TOF_QdcCalibMethod::fGetEnergy };

	  std::map< uint32_t, TOF_CoincidenceChannelInfo> fChannelDataList; // {nullptr};
		std::vector< uint32_t > fActiveChannelList;

	public:
		int  setInputPathStg2( const char* fnameStg2 );
		int  setTreeData( TOF_TreeDataStg1* tr );// { fTree = tr; };
		int  setTreeData( TOF_TreeDataStg2* tr );// { fTree = tr; };
		void setTriggerChannel( uint32_t trigCh ) { fTrigChannelID = trigCh; }; // coincidence evt using trigger channel is not prepared yet
		void setActiveChannels( std::vector<uint32_t> chanList );
		void setCoincidenceTimeWindowInClk( double twindow ) { fCoinTimeWindow = twindow; }; // in clock

		TTree* getCoincidenceEventsTree();
		//std::vector<std::vector<TOF_CoincidenceChannelInfo>> getCoincidenceEvents();

		void createCoinTree();
		void reset();

	public:
		void setQdcCalibMethod( TOF_QdcCalibMethod calMethod ) { fQdcCalibMethod = calMethod; };

	
	/// for QA
	private:
		const double fTdcFreq = 200E6; // 200 MHz - need to doublecheck it
		const double fTdcClk = 1./fTdcFreq; // sec
		const double fTdcClkNs = fTdcClk/pow(10,-9); // ns

		TH1D* fHisto_dT{nullptr}; // time resolution (ns)
		TH1D* fHisto_NbOfEvt{nullptr};
	  TH2D* fHisto_TvsQcal{nullptr}; // = new TH2D("hTvsQcal", ";Time diff in clock;", 300, -3, 3, 100, -0.6, 0.6);
	
	public:
		void generateHistoForQA(const char* pdfName);
		TH1D* getHisto_TimeResol()       { return fHisto_dT     ; };
		TH1D* getHisto_ChannelVsNevents(){ return fHisto_NbOfEvt; };
		TH2D* getHisto_TdcVsQcalib()     { return fHisto_TvsQcal; };


	ClassDef(TOF_CoincidenceEvents, 1)

};





