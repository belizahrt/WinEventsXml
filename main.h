#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <fstream>
#include <codecvt>
#include <sstream>
#include <chrono>

#include "teebuf.hpp"

#include "winevt\winevtwrapper.h"
#include "winevt\winevtmerger.h"
using namespace wew;

namespace Main {

    const wchar_t* SplitLine = 
        L"-------------------------------------------\n";

    const short RewriteOutput = 0;
    const short MergeOutput = 1;

    struct MainInfo {
        std::wstring	remoteComputer{};
        std::wstring	queryFile{};
        std::wstring	outputFile{};
        short           outputMode{ RewriteOutput };

        bool            makeLog;
        std::wstring    logPath;

        bool            makeBackup;
        std::wstring    backupPath;
    };

    std::wstring GetEvents(const std::wstring& computer,
        const std::wstring& queryFile);
    void OutputEvents(const std::wstring& newEvents,
        const std::wstring& outputFile,
        short mode);

    void OutputExecutionInfo(const Main::MainInfo& mainInfo);

    void OutputErrors(const ErrorsVector& errors);

    void OutputLogo();
    void OutputHelp();

    std::wstring StringToWString(const std::string& s);
    std::wstring GetCurrentComputerName();
    MainInfo HandleArgs(int argc, char** argv);
}

template <typename T>
struct FileLoggingHelper 
{
    FileLoggingHelper(std::basic_ostream<T>& basicStream,
        const std::wstring logPath, int openMode = std::ios::app)
        : m_basicStream(&basicStream), m_stdBuf(m_basicStream->rdbuf())
    {
        m_logFile.open(logPath, openMode);
        m_teeBuf = new basic_teebuf<T>(&m_logFile, m_stdBuf);
        m_basicStream->rdbuf(m_teeBuf);
    }

    ~FileLoggingHelper()
    {
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
        m_basicStream->rdbuf(m_stdBuf);
        delete m_teeBuf;
    }

private:
    std::basic_ostream<T>*      m_basicStream;
    std::basic_streambuf<T>*    m_stdBuf;
    std::basic_filebuf<T>       m_logFile;
    basic_teebuf<T>*            m_teeBuf;
};

using WFileLogger = FileLoggingHelper<wchar_t>;

struct ScopeTimeElapser 
{
    ScopeTimeElapser()
        : start(std::chrono::steady_clock::now()) {}

    ~ScopeTimeElapser() {
        auto end = std::chrono::steady_clock::now();
        auto diff =
            std::chrono::duration_cast<std::chrono::milliseconds>
            (end - start).count();

        std::wclog << L"Duration: " << diff << L" msec\n";
    }

private:
    std::chrono::steady_clock::time_point start;
};

#endif