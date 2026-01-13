#include "process_tofCoinEvtQA.h"
#include <iostream>
#include <getopt.h>

int main(int argc, char** argv) {
    std::string inputFile;
    std::string outputBase;
    std::string tdcCalib;
    std::string qdcCalib;
    int qdcMode = 1;

    static struct option longOptions[] = {
        { "help",      no_argument,       0, 'h' },
        { "input",     required_argument, 0, 'i' },
        { "output",    required_argument, 0, 'o' },
        { "qdcmode",   required_argument, 0, 'm' },
        { "tdccal",    required_argument, 0, 't' },
        { "qdccal",    required_argument, 0, 'q' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "hi:o:m:t:q:", longOptions, NULL)) != -1) {
        switch (c) {
            case 'i': inputFile = optarg; break;
            case 'o': outputBase = optarg; break;
            case 'm': qdcMode = std::stoi(optarg); break;
            case 't': tdcCalib = optarg; break;
            case 'q': qdcCalib = optarg; break;
            case 'h':
            default:
                std::cout << "Usage: ./tofCoinEvt -i <input.root> -o <out_base> [options]\n";
                return 0;
        }
    }

    if (inputFile.empty() || outputBase.empty()) {
        std::cerr << "Error: Input and Output are required.\n";
        return 1;
    }

    if (!runProcessTofCoinEvtQA(inputFile, outputBase, qdcMode, tdcCalib, qdcCalib)) {
        std::cerr << "Analysis failed.\n";
        return 1;
    }

    return 0;
}
