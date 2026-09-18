//#include "process_tofCoinEvtQA.h"
#include "process_convertStg1ToStg2.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char** argv) {
    std::string inputFile;
    std::string outputBase = "";
    std::string tdc_cal_tsv= "";
    std::string qdc_cal_tsv= "";

    static struct option longOptions[] = {
        { "help"            ,  no_argument     , 0, 'h' },
        { "input"           , required_argument, 0, 'i' },
        { "output"          , optional_argument, 0, 'o' },
        { "tdc_calibration" , optional_argument, 0, 't' },
        { "qdc_calibration" , optional_argument, 0, 'q' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "hi:o:t:q:a::", longOptions, NULL)) != -1) {
        switch (c) {
            case 'i': if(optarg) inputFile     = optarg; break;
            case 'o': if(optarg) outputBase    = optarg; break;
            case 't': if(optarg) tdc_cal_tsv   = optarg; break;
            case 'q': if(optarg) qdc_cal_tsv   = optarg; break;
            case 'h':
            default:
                std::cout << "Usage: ./process_convertStg1ToStg2 -i <input.stg1.root> -o <out_base> -t <tdc_calibration.tsv> -q <qdc_calibration.tsv>\n";
                return 0;
				}
    }

    if (inputFile.empty() ) {
       std::cerr << "[ERR] Input is required.\n";
       return 1;
    }
    if ( tdc_cal_tsv.empty() ) {
        std::cerr << "[WARN] TDC calibration file is loaded from Config directory." << std::endl;
    }
    if ( qdc_cal_tsv.empty() ) {
        std::cerr << "[WANR] QDC calibration file is loaded from Config directory." << std::endl;
    }
	
    if (outputBase.empty()) {
        std::cout << "[WANR] Output File Path is not given. Use default.\n";
    }
    
		if (!runConvertStg1ToStg2(inputFile, outputBase, tdc_cal_tsv, qdc_cal_tsv)) {
	    std::cerr << "[ERR] Analysis failed.\n";
			return 1;
    }

    return 0;
}
