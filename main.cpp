#include "main.h"

using namespace Main;

void CreateOutputBackup(const std::wstring& outputFilePath,
    const std::wstring& backupPath)
{
    CopyFile(outputFilePath.c_str(), backupPath.c_str(), false);
}

int main(int argc, char** argv)
{
    OutputLogo();

    MainInfo mainInfo = HandleArgs(argc, argv);

    std::unique_ptr<WFileLogger> logHelper;
    if (mainInfo.makeLog) {
        logHelper.reset(
            new WFileLogger(std::wclog, mainInfo.logPath, std::ios::app));
    }

    if (mainInfo.makeBackup) {
        CreateOutputBackup(mainInfo.outputFile, mainInfo.backupPath);
    }

    ScopeTimeElapser timeElapsed;
    
    OutputExecutionInfo(mainInfo);
    std::wclog << SplitLine;

    try {
        std::wclog << "Executing query...\n";
        std::wstring eventsRetrieved = 
            GetEvents(mainInfo.remoteComputer, mainInfo.queryFile);
        std::wclog << L"Executing query has been finished.\n";

        Main::OutputEvents(eventsRetrieved, mainInfo.outputFile, 
            mainInfo.outputMode);
    }
    catch (const wchar_t* message) {
        std::wclog << message << std::endl;
    }

    std::wclog << std::hex << L"Finished. (LEC=0x" << GetLastError() << L")\n"
        << std::dec;
    std::wclog << SplitLine;

    return 0;
}

std::wstring Main::GetEvents(const std::wstring& computer,
    const std::wstring& queryFile)
{
    WinEvtXmlWrapper winEvtXml;
    winEvtXml.SetComputer(computer);
    
    if (!winEvtXml.LoadQueryFromFile(queryFile)) {
        
        throw L"Cannot open query file or file is invalid!";
    }
    
    std::wstring eventsRetrieved = winEvtXml.ExecuteQuery();
    if (eventsRetrieved.length() == 0) {
        OutputErrors(winEvtXml.GetErrors());
        
        throw L"Retrieving events failed.";
    }

    return eventsRetrieved;
}

void Main::OutputEvents(const std::wstring& newEvents,
    const std::wstring& outputFile, 
    short mode = Main::MergeOutput)
{
    WinEvtXmlMerger winEvtMerger;
    winEvtMerger.setIEventsString(newEvents);

    if (mode == Main::MergeOutput) {
        std::wclog << L"Merging events in file.\n";

        winEvtMerger.loadOEventsFile(outputFile.c_str());
        DeleteFile(outputFile.c_str());
    }
    else if (mode == Main::RewriteOutput) {
        std::wclog << L"Writing events in file.\n";

        winEvtMerger.setOEventsString({});
    }

    auto [mergedEvents, insertedCount] = winEvtMerger.Merge();
    std::wclog << insertedCount << L" events inserted.\n";

    std::wclog << "Saving in " << outputFile.c_str() << std::endl;
    std::wclog << (mergedEvents.save_file(outputFile.c_str()) ?
        L"...succeed" : L"...failed") << std::endl;
}

void Main::OutputExecutionInfo(const Main::MainInfo& mainInfo)
{
    std::wclog << L"\nExecution log for " << mainInfo.remoteComputer << L" on ";
    std::wclog << GetCurrentComputerName() << std::endl;

    auto timeNow = std::chrono::system_clock::now();
    std::time_t timeNowT = std::chrono::system_clock::to_time_t(timeNow);

    std::wclog << _wctime(&timeNowT);
}

void Main::OutputErrors(const ErrorsVector& errors)
{
    if (errors.size() > 0) {
        std::wclog << L"Errors procced:" << std::endl;
        for (auto error : errors) {
            std::wclog << error.second << std::endl <<
                L"Last error code: " << error.first << std::endl;
        }
    }
}

void Main::OutputLogo()
{
    std::cout << "/-----------------------------------\\" << std::endl;
    std::cout << "|  Windows Events Xml Extractor     |" << std::endl;
    std::cout << "|  github: /belizahrt/WinEventsXml  |" << std::endl;
    std::cout << "|  mail:   kua@amur.so-ups.ru       |" << std::endl;
    std::cout << "\\-----------------------------------/" << std::endl;
    std::cout << std::endl;
}

void Main::OutputHelp()
{
    std::cout << "Usage: WinEventsXml.exe [COMPUTER] [QUERY_FILE_PATH] [OUTPUT_FILE_PATH] [-m|-r| ] ";
    std::cout << "[/log <path>] [/bu <path>]\n\n";
    std::cout << "-m - merge output mode" << std::endl;
    std::cout << "-r or empty - rewrite output mode" << std::endl;
    std::cout << "/log <path> - writing log in file" << std::endl;
    std::cout << "/bu <path> - create backup of output xml file" << std::endl;
    std::cout << "------------------------------------------------------------------------" 
        << std::endl;
    std::cout << "Use ONLY UTF encoding in query file!" << std::endl;
}

Main::MainInfo Main::HandleArgs(int argc, char** argv)
{
    Main::MainInfo mainInfo;

    if (argc >= 4) {
        if (argv[1] != nullptr) {
            mainInfo.remoteComputer = StringToWString(std::string(argv[1]));
        }
        if (argv[2] != nullptr) {
            mainInfo.queryFile = StringToWString(std::string(argv[2]));
        }
        if (argv[3] != nullptr) {
            mainInfo.outputFile = StringToWString(std::string(argv[3]));
        }

        for (int i = 4; i < argc; ++i) {
            if (argv[i] != nullptr) {
                std::string arg(argv[i]);

                if (arg == "/log") {
                    mainInfo.makeLog = true;
                    mainInfo.logPath = 
                        (argv[i + 1] == nullptr) ? L"" : 
                        StringToWString(argv[i + 1]);
                }
                if (arg == "-m") {
                    mainInfo.outputMode = MergeOutput;
                }
                if (arg == "/bu") {
                    mainInfo.makeBackup = true;
                    mainInfo.backupPath = 
                        (argv[i + 1] == nullptr) ? L"" : 
                        StringToWString(argv[i + 1]);
                }
            }
        }
    }
    else {
        OutputHelp();
    }

    return mainInfo;
}

// ---------------------------------------------------
// utils:

// only for 1 byte chars
std::wstring Main::StringToWString(const std::string& s)
{
    std::wstring wsTmp(s.begin(), s.end());

    return wsTmp;
}

std::wstring Main::GetCurrentComputerName()
{
    DWORD MaxComputerNameLength = 256;
    wchar_t* computerName = new wchar_t[MaxComputerNameLength];
    GetComputerNameW(computerName, &MaxComputerNameLength);

    return { computerName };
}