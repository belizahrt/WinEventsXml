#ifndef SLA_H
#define SLA_H

#include <iostream>
#include <fstream>
#include <codecvt>
#include <sstream>

#include "winevt\winevtwrapper.h"
#include "winevt\winevtmerger.h"
using namespace wew;

namespace Main {

    const short RewriteOutput = 0;
    const short MergeOutput = 1;

    struct MainInfo {
        std::wstring	remoteComputer{};
        std::wstring	queryFile{};
        std::wstring	outputFile{};
        short			outputMode{ RewriteOutput };
    };

    std::wstring GetEvents(const std::wstring& computer,
        const std::wstring& queryFile);
    void OutputEvents(const std::wstring& newEvents,
        const std::wstring& outputFile,
        short mode);

    void OutputErrors(const ErrorsVector& errors);

    void OutputLogo();
    void OutputHelp();

    int StringToWString(std::wstring& ws, const std::string& s);
    MainInfo HandleArgs(int argc, char** argv);

}

#endif