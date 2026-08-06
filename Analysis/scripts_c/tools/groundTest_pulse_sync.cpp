#include "groundTest_pulse_sync.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char** argv) {
    std::string inputFile;
    std::string outputBase;
		int febD_connID   = -1;
		int febS_connID_A = -1;
		int febS_connID_B = -1;

    static struct option longOptions[] = {
        { "help",      no_argument,       0, 'h' },
        { "input",     required_argument, 0, 'i' },
        { "output",    required_argument, 0, 'o' },
        { "febD_connID", required_argument, 0, 'c' },
        { "febS_connID_A", required_argument, 0, 'a' },
        { "febS_connID_B", required_argument, 0, 'b' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "hi:o:c:a:b:", longOptions, NULL)) != -1) {
        switch (c) {
            case 'i': inputFile     = optarg; break;
            case 'o': outputBase    = optarg; break;
						case 'c': febD_connID   = std::atoi(optarg); break;
            case 'a': febS_connID_A = std::atoi(optarg); break;
            case 'b': febS_connID_B = std::atoi(optarg); break;
            case 'h':
            default:
               std::cout << "Usage: ./groundTest_pulse_sync -i <input.stg1.root> -o <out_base> [options]\n";
                return 0;
        }
    }

    if (inputFile.empty() ) {
        std::cerr << "[ERR] Input is required.\n";
        return 1;
    }
    if (outputBase.empty()) {
        std::cout << "[WARN] Output File Path is not given. Use default.\n";
      if (!runGroundTestPulseSync(inputFile, "", febD_connID, febS_connID_A, febS_connID_B)) {
          std::cerr << "[ERR] Analysis failed.\n";
          return 1;
      }
		}
		else {
      if (!runGroundTestPulseSync(inputFile, outputBase, febD_connID, febS_connID_A, febS_connID_B)) {
          std::cerr << "[ERR] Analysis failed.\n";
          return 1;
      }
		}

    return 0;
}
