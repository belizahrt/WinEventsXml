#include "main.h"

int main(int argc, char** argv)
{
    Main::OutputLogo();

    Main::MainInfo mainInfo = Main::HandleArgs(argc, argv);

    try {
        std::wclog << "Executing query...\n";
        std::wstring eventsRetrieved = 
            Main::GetEvents(mainInfo.remoteComputer, mainInfo.queryFile);
        std::wclog << L"Executing query has been finished.\n";

        Main::OutputEvents(eventsRetrieved, mainInfo.outputFile, 
            mainInfo.outputMode);
    }
    catch (const wchar_t* message) {
        std::wcerr << message;
    }

    std::wclog << std::hex << L"Finished. (LEC=0x" << GetLastError() << L")\n";

    return 0;
}

std::wstring Main::GetEvents(const std::wstring& computer,
    const std::wstring& queryFile)
{
    WinEvtXmlWrapper winEvtXml;
    winEvtXml.SetComputer(computer);
    winEvtXml.LoadQueryFromFile(queryFile);
    
    std::wstring eventsRetrieved = winEvtXml.ExecuteQuery();
    if (eventsRetrieved.length() == 0) {

        throw L"Retrieving events failed.";

        OutputErrors(winEvtXml.GetErrors());
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
    std::cout << "|  kua@amur.so-ups.ru               |" << std::endl;
    std::cout << "\\-----------------------------------/" << std::endl;
    std::cout << std::endl;
}

void Main::OutputHelp()
{
    std::cout << "Usage: sla.exe [COMPUTER] [QUERY_FILE_PATH] [OUTPUT_FILE_PATH] [-m|-r| ]"
        << std::endl;
    std::cout << "-m - merge output mode" << std::endl;
    std::cout << "-r or empty - rewrite output mode" << std::endl;
    std::cout << "------------------------------------------------------------------------" 
        << std::endl;
    std::cout << "Use ONLY UTF encoding in query file!" << std::endl;
}

Main::MainInfo Main::HandleArgs(int argc, char** argv)
{
    Main::MainInfo mainInfo;

    if (argc >= 3) {
        if (argv[1] != nullptr) {
            StringToWString(mainInfo.remoteComputer, std::string(argv[1]));
        }
        if (argv[2] != nullptr) {
            StringToWString(mainInfo.queryFile, std::string(argv[2]));
        }
        if (argv[3] != nullptr) {
            StringToWString(mainInfo.outputFile, std::string(argv[3]));
        }

        if (argv[4] != nullptr) {
            if (!strcmp(argv[4], "-m")) {
                mainInfo.outputMode = MergeOutput;
            }
            else {
                mainInfo.outputMode = RewriteOutput;
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
int Main::StringToWString(std::wstring& ws, const std::string& s)
{
    std::wstring wsTmp(s.begin(), s.end());

    ws = wsTmp;

    return 0;
}