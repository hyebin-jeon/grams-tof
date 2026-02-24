#include "process_tofQA_iridium.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char** argv) {
    std::string inputFile="";
    std::string outputBase="";
    std::string asicListFile="";
		//int runTimeSec = -1;

    static struct option longOptions[] = {
        { "help",      no_argument,       0, 'h' },
        { "input",     required_argument, 0, 'i' },
        { "output",    required_argument, 0, 'o' },
        { "asic_list", required_argument, 0, 'a' },
        { "run_time" , required_argument, 0, 't' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "hi:o:a:t:", longOptions, NULL)) != -1) {
        switch (c) {
            case 'i': inputFile    = optarg; break;
            case 'o': outputBase   = optarg; break;
            case 'a': asicListFile = optarg; break;
						//case 't': runTimeSec   = std::atoi(optarg); break;
            case 'h':
            default:
                std::cout << "Usage: ./process_tofQA_iridium -i <input.stg1.root> -o <out_base> -a <active_asic_list.tsv> -t <run_time_in_sec> \n";
                return 0;
        }
    }

    if (inputFile.empty()) {
        std::cerr << "[ERR] Input is required.\n";
        return 1;
    }
    if (asicListFile.empty()) {
        std::cerr << "[ERR] active_asic_list.tsv is required.\n";
        return 1;
		}
    if (outputBase.empty()) {
        std::cout << "[WARN] Output File Path is not given. Use default.\n";
		}
      
		//if (!runTofQA_Iridium(inputFile, outputBase, asicListFile, runTimeSec )) {
		if (!runTofQA_Iridium(inputFile, outputBase, asicListFile )) {
			std::cerr << "[ERR] Analysis failed.\n";
			return 1;
		}

    return 0;
}
