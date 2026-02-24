#include "process_tofQA_iridium.h"

#include <TH1F.h>
#include <TCanvas.h>
#include <TString.h>
#include <TStyle.h>
#include <TFile.h>
#include <TOF_Constants.h>
#include <TOF_Attributes.h>
#include <TOF_ChannelConversion.h>
#include <TOF_TreeDataStg2.h>

bool runTofQA_Iridium( const std::string& inputFile,
                       const std::string& outputBase,
											 const std::string& asicListFile
											 //const int runTimeSec_
											 )
{
	/// output naming
  const char* inputFile_c = inputFile.c_str();	
	
	/// channel conversion class
	auto theAttrib = TOF_Attributes::getInstance();

	/// channel conversion class
	auto theChanConv = TOF_ChannelConversion::getInstance();
	if( theChanConv->readActiveAsicList(asicListFile.c_str())!= TOF_GOOD ) return false;
	auto activeConnIds_D = theChanConv->getActiveConnIdOnFebD();
	
	const int nb = 2;

	if( activeConnIds_D.size() != nb ) {
		std::cerr << "[WARN] Number of Active Asic ConnectorID List != 2" << std::endl;
	}

	/// input stg2
	TOF_TreeDataStg2* stg2 = new TOF_TreeDataStg2();
	stg2->setInputPath( inputFile_c );
	stg2->setBranchAddress();

	/// x-axis time range
	const int binW = 10; // in sec
	stg2->getEntry(0);
	TTimeStamp t_begin = stg2->getTimeStampCPU();
	stg2->getEntry( stg2->getEntries()-1 );
	TTimeStamp t_end = stg2->getTimeStampCPU();
	int tmin = t_begin.GetSec();
	int dur_nano = t_end.GetNanoSec() - t_begin.GetNanoSec();
	int dur  = t_end.GetSec() - t_begin.GetSec();
	if( dur_nano < 0 ) dur = dur -1;
	else if( dur_nano > 0 ) dur = dur + 1;
	int tmax = dur%binW==0? t_end.GetSec() : t_end.GetSec()+ binW;

	const double runTimeSec = (t_end.GetSec() + (double)t_end.GetNanoSec()*1E-9) - (t_begin.GetSec() + (double)t_begin.GetNanoSec()*1E-9);;

	/// histograms
	int timeRange = tmax - tmin;
	int timeBinNb  = timeRange%10? timeRange/10+1 : timeRange/10;
	TH1F* hEvtChan[nb];
	TH1F* hEvtTime[nb];
	for( int i=0; i<nb; i++ )
	{
	  hEvtChan[i] = new TH1F(Form("hEvtChan%d",i), Form("Channel vs. Event rate (FebD_%02d)", activeConnIds_D[i]), 128, 1, 129 );
	  hEvtTime[i] = new TH1F(Form("hEvtTime%d",i), Form("CPU time vs. Event rate (FebD_%02d)", activeConnIds_D[i]), timeBinNb, tmin, tmax); // 1800 sec = 40 min, 1 bin per 10 sec

		hEvtChan[i]->GetXaxis()->SetTitle("connector ID on FEB-S");
		hEvtTime[i]->GetXaxis()->SetTitle("CPU time (UTC), 10 sec/bin");
		hEvtChan[i]->GetYaxis()->SetTitle("Event rate (Hz)");
		hEvtTime[i]->GetYaxis()->SetTitle("Event rate (Hz)");
	}

	/// fill the histo
	for( int i=0; i<stg2->getEntries(); i++)
	{
		stg2->getEntry(i);

		auto ts_pps   = stg2->getTimeStampPPS();
		auto connID_D = stg2->getConnID_FebD();
		auto connID_S = stg2->getConnID_FebS();

		for( int j=0; j<nb; j++ )
		{
		  if( connID_D != activeConnIds_D[j] ) continue;
			
			hEvtChan[j]->Fill( connID_S, 1./ runTimeSec );
			//hEvtChan[j]->Fill( connID_S );
			hEvtTime[j]->Fill( ts_pps.AsDouble(), 1./(double)binW );
		}
	}


	/// output file naming
	std::string fin_root = stg2->getFileName();
	std::string fin_dir  = std::filesystem::path(inputFile_c).parent_path();
	auto        fin_idx  = ((TString) fin_root).Index(".root");
	TString     fin_name = ((TString) fin_root)(0, fin_idx);

	std::string fout_dir = outputBase.empty()==true? fin_dir :  outputBase;
	std::string fout_suffix = "iridiumQA";
	const char* fout_name = Form("%s/%s.%s", fout_dir.c_str(), fin_name.Data(), fout_suffix.c_str());
	const char* fout_root = Form("%s.root", fout_name);
	const char* fout_pdf  = Form("%s.pdf" , fout_name);

	TFile* fout = new TFile( fout_root, "recreate");

	TCanvas* canv0 = new TCanvas("canv0", "canv0"); //, 1100, 500);
	canv0->Print( Form("%s[", fout_pdf) ); // open 
	canv0->Divide(nb,2,0.005,0.005);

	gStyle->SetOptStat(111111);
	for( int j=0; j<nb; j++ )
	{
	  /// scale the histo to make y axis = event rate
		//hEvtChan[j]->Scale( 1./(double) runTimeSec );

		theAttrib->attribHist( hEvtChan[j] );
		theAttrib->attribHist( hEvtTime[j] );
		hEvtTime[j]->GetXaxis()->SetTimeDisplay(1);
	  hEvtTime[j]->GetXaxis()->SetTimeFormat ("%m/%d %H:%M");
	  hEvtTime[j]->GetXaxis()->SetTimeOffset(0, "gmt");

	  canv0->cd(j+1);
	  hEvtChan[j]->Draw("hist");
	  canv0->cd(j+3);
	  hEvtTime[j]->Draw("hist");

		hEvtChan[j]->Write();
		hEvtTime[j]->Write();
	}

	canv0->Print( fout_pdf );
	canv0->Print( Form("%s]", fout_pdf) ); // close

	fout->Close();

	std::cout << "[INFO] Output (root): " << fout->GetName() << std::endl;
	std::cout << "[INFO] Output (pdf ): " << fout_pdf << std::endl;

	return true;

}
