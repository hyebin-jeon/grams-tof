#include "process_discSetting.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char** argv) {
    std::string inputFile;
    std::string outputBase;

    static struct option longOptions[] = {
        { "help",      no_argument,       0, 'h' },
        { "input",     required_argument, 0, 'i' },
        { "output",    required_argument, 0, 'o' },
        { "disc"  ,    required_argument, 0, 'd' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "hi:o:d:", longOptions, NULL)) != -1) {
        switch (c) {
            case 'i': inputFile = optarg; break;
            case 'o': outputBase = optarg; break;
            case 'h':
            default:
                std::cout << "Usage: ./process_darkCount -i <disc_calibration.tsv> -o <out_base> \n";
                return 0;
        }
    }

    if (inputFile.empty() ) {
        std::cerr << "[ERR] Input is required.\n";
        return 1;
    }

    if (outputBase.empty()) {
        std::cout << "[WARN] Output File Path is not given. Use default.\n";
    }

		if (!runProcessDiscSetting(inputFile, outputBase)) {
			std::cerr << "[ERR] Analysis failed.\n";
      return 1;
    }

    return 0;
}
