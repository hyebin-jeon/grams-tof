#pragma once

#include "TObject.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include <array>

#ifndef TOF_FITTING_H
#define TOF_FITTING_H

class TOF_Fitting : public TObject
{
	public: 
    inline static TOF_Fitting * theFit{nullptr}; 
  	static TOF_Fitting *getInstance() {
  		if( theFit == nullptr ) {
  			theFit = new TOF_Fitting;
  		}
  
  		return theFit;
  	};
  
  	~TOF_Fitting() = default;

	public:
		void fitGauss  ( TH1* h, double Nsigma );
		void fitLorentz( TH1* h, double fitR );
		void fitLinear ( TH2* h, double fitXmin, double fitXmax );

		TF1* getFitFunction() { return fFunc; };

	private:
		TF1* fFunc{nullptr};
		double getXatYmax( TH1* h );
		std::array<Double_t,2> GetX_withY( TH1* h, Double_t ywant );
	  
		ClassDef(TOF_Fitting, 1)

};

#endif

