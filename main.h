#ifndef SLA_H
#define SLA_H

#include <iostream>
#include <fstream>
#include <codecvt>
#include <sstream>

#include "winevt\winevtwrapper.h"
#include "winevt\winevtmerger.h"
using namespace wew;

const short RewriteOutput = 0;
const short MergeOutput = 1;

struct MainInfo {
    std::wstring	remoteComputer {};
    std::wstring	queryFile {};
    std::wstring	outputFile {};
    short			outputMode{ RewriteOutput };
};

void OutputErrors(const ErrorsVector& errors);

void OutputLogo();
void OutputHelp();

int StringToWString(std::wstring& ws, const std::string& s);
std::wstring readFile(const wchar_t* path);
MainInfo HandleArgs(int argc, char** argv);

#endif