//#include "process_tofCoinEvtQA.h"
#include "process_convertStg1ToStg2.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char** argv) {
    std::string inputFile;
    std::string outputBase;

    static struct option longOptions[] = {
        { "help",      no_argument,       0, 'h' },
        { "input",     required_argument, 0, 'i' },
        { "output",    required_argument, 0, 'o' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "hi:o:", longOptions, NULL)) != -1) {
        switch (c) {
            case 'i': inputFile = optarg; break;
            case 'o': outputBase = optarg; break;
            case 'h':
            default:
                std::cout << "Usage: ./process_convertStg1ToStg2 -i <input.stg1.root> -o <out_base> [options]\n";
                return 0;
        }
    }

    if (inputFile.empty() ) {
        std::cerr << "[ERR] Input is required.\n";
        return 1;
    }
    if (outputBase.empty()) {
        std::cout << "[WARN] Output File Path is not given. Use default.\n";
      if (!runConvertStg1ToStg2(inputFile)) {
          std::cerr << "[ERR] Analysis failed.\n";
          return 1;
      }
		}
		else {
      if (!runConvertStg1ToStg2(inputFile, outputBase)) {
          std::cerr << "[ERR] Analysis failed.\n";
          return 1;
      }
		}

    return 0;
}
