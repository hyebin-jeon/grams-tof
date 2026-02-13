#include "process_tofCoinEvtQA.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char** argv) {
    std::string inputFile;
    std::string outputBase;
    std::string tdcCalib;
    std::string qdcCalib;
		int febD_connID = -1;

    static struct option longOptions[] = {
        { "help",        no_argument,       0, 'h' },
        { "input",       required_argument, 0, 'i' },
        { "output",      required_argument, 0, 'o' },
        { "tdccal",      required_argument, 0, 't' },
        { "qdccal",      required_argument, 0, 'q' },
        { "febD_connID", required_argument, 0, 'c' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "hi:o:t:q:c:", longOptions, NULL)) != -1) {
        switch (c) {
            case 'i': inputFile = optarg; break;
            case 'o': outputBase = optarg; break;
            case 't': tdcCalib = optarg; break;
            case 'q': qdcCalib = optarg; break;
						case 'c': febD_connID = std::atoi(optarg); break;
            case 'h':
            default:
                std::cout << "Usage: ./process_tofCoinEvtQA -i <input.root> -o <out_base> [options]\n";
                return 0;
        }
    }

    if (inputFile.empty() ) {
        std::cerr << "[ERR] Input is required.\n";
        return 1;
    }
    if (tdcCalib.empty() || qdcCalib.empty()) {
        std::cerr << "[ERR] TDC/QDC calibration iles are required.\n";
        return 1;
    }


    if (outputBase.empty()) {
        std::cout << "[WARN] Output File Path is not given. Use default.\n";
      if (!runProcessTofCoinEvtQA(inputFile, "", tdcCalib, qdcCalib, febD_connID)) {
          std::cerr << "[ERR] Analysis failed.\n";
          return 1;
      }
		}
		else {
      if (!runProcessTofCoinEvtQA(inputFile, outputBase, tdcCalib, qdcCalib, febD_connID)) {
          std::cerr << "[ERR] Analysis failed.\n";
          return 1;
      }
		}

    return 0;
}
