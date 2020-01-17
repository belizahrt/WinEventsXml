#include "main.h"

int main(int argc, char** argv)
{
    OutputLogo();

    MainInfo mainInfo = HandleArgs(argc, argv);

    WinEvtXmlWrapper winEvtXml;
    winEvtXml.SetComputer(mainInfo.remoteComputer);
    winEvtXml.LoadQueryFromFile(mainInfo.queryFile);

    std::wstring eventsRetrieved = winEvtXml.ExecuteQuery();
    if (eventsRetrieved.length() == 0) {
        
        std::clog << "Retrieving events failed.\n";

        OutputErrors(winEvtXml.GetErrors());
        return 0;
    }

    std::clog << "Executing query has been finished.\n";

    if (mainInfo.outputMode == MergeOutput) {
        std::clog << "Merging events.\n";
        std::wstring outputEvents = readFile(mainInfo.outputFile.c_str());
        WinEvtXmlMerger evtMerger(eventsRetrieved, outputEvents);
        evtMerger.GetXmlEvents().save_file(mainInfo.outputFile.c_str());
    }
    else if (mainInfo.outputMode == RewriteOutput) {
        std::clog << "Writing events in file.\n";
        WinEvtXmlMerger evtMerger(eventsRetrieved, L"");
        evtMerger.GetXmlEvents().save_file(mainInfo.outputFile.c_str());
    }

    return 0;
}

void OutputErrors(const ErrorsVector& errors)
{
    if (errors.size() > 0) {
        std::wclog << L"Errors procced:" << std::endl;
        for (auto error : errors) {
            std::wclog << error.second << std::endl <<
                L"Last error code: " << error.first << std::endl;
        }
    }
}

void OutputLogo()
{
    std::cout << "/--------------------------------\\" << std::endl;
    std::cout << "|      SysLogAnalysis  v2.0      |" << std::endl;
    std::cout << "|       kua@amur.so-ups.ru       |" << std::endl;
    std::cout << "\\--------------------------------/" << std::endl;
    std::cout << std::endl;
}

void OutputHelp()
{
    std::cout << "Usage: sla.exe [COMPUTER] [QUERY_FILE_PATH] [OUTPUT_FILE_PATH] [-m|-r| ]"
        << std::endl;
    std::cout << "-m - merge output mode" << std::endl;
    std::cout << "-r or empty - rewrite output mode" << std::endl;
    std::cout << "------------------------------------------------------------------------" 
        << std::endl;
    std::cout << "Use ONLY UTF encoding in query file!" << std::endl;
}

MainInfo HandleArgs(int argc, char** argv)
{
    MainInfo mainInfo;

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

std::wstring readFile(const wchar_t* path)
{
    std::wifstream wif(path);
    wif.imbue(std::locale(std::locale::empty(), 
        new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << wif.rdbuf();
    wif.close();
    return wss.str();
}

// only for 1 byte chars
int StringToWString(std::wstring& ws, const std::string& s)
{
    std::wstring wsTmp(s.begin(), s.end());

    ws = wsTmp;

    return 0;
}