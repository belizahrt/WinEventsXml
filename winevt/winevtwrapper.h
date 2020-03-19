#ifndef WINEVTWRAPPER_H
#define WINEVTWRAPPER_H

#include "winevt.h"

#include <iostream>
#include <fstream>
#include <codecvt>
#include <sstream>
#include <vector>
#include <Windows.h>
#include <sddl.h>

namespace wew {

    const DWORD 
    DefaultQueryFlags{ EvtQueryChannelPath | EvtQueryReverseDirection };
    const DWORD EventsPackCount{ 10 };

    using 
    ErrorsVector = std::vector<std::pair<DWORD, std::wstring>>;

    class WinEvtXmlWrapper final {
    public:
        WinEvtXmlWrapper() = default;
        WinEvtXmlWrapper operator=(const WinEvtXmlWrapper&) = delete;
        WinEvtXmlWrapper(const WinEvtXmlWrapper&) = delete;
        
        std::wstring    ExecuteQuery();
        bool            LoadQueryFromFile(const std::wstring& path);
        
        std::wstring    GetComputer() const;
        void            SetComputer(const std::wstring& value);
        std::wstring    GetQuery() const;
        void            SetQuery(const std::wstring& value);
        DWORD           GetQueryFlags() const;
        void            SetQueryFlags(const DWORD value);
        ErrorsVector    GetErrors() const;

    private:
        EVT_HANDLE      m_hSession{ 0 };
        std::wstring    m_computer;
        std::wstring    m_query;
        DWORD           m_queryFlags{ DefaultQueryFlags };
        ErrorsVector    m_Errors;

        EVT_HANDLE      OpenRemoteSession(LPCWSTR lpwszRemote);
        EVT_HANDLE      CreateProviderRenderContext();
        std::wstring    FetchEvents(EVT_HANDLE hEvents);
        std::wstring    RenderEventToXml(EVT_HANDLE hEvent);
        LPVOID          Render(EVT_HANDLE hContext, EVT_HANDLE hEvent,
                            EVT_RENDER_FLAGS flags);
        LPWSTR          FormatEventMessageString(EVT_HANDLE hMetadata,
                            EVT_HANDLE hEvent);
        void            PushError(const DWORD lastErrorCode, 
                            const wchar_t* message);
    };

    struct WinEvtXmlException {
        DWORD           lastErrorCode;
        const wchar_t*  message;
    };

    struct EvtHandleDeleter {
        void operator()(EVT_HANDLE* handle) {
            if ((*handle)) {
                EvtClose(*handle);
            }
        }
    };
    using EvtHandleHolder = std::unique_ptr<EVT_HANDLE, EvtHandleDeleter>;

    struct EvtRenderCaller {
        bool operator()();

        EVT_HANDLE          hContext;
        EVT_HANDLE          hEvent;
        EVT_RENDER_FLAGS    dwFlags{ EvtRenderEventValues };
        DWORD               dwBufferSize;
        LPVOID              pRenderedValues;
        DWORD               dwBufferUsed; 
        DWORD               dwPropertyCount;
    };

    struct EvtFormatMessageCaller {
        bool operator()();

        EVT_HANDLE      hMetadata;
        EVT_HANDLE      hEvent;
        DWORD           dwMessageId;
        DWORD           dwValuesCount;
        PEVT_VARIANT    pRenderedValues;
        DWORD           dwFlags{ EvtFormatMessageXml };
        DWORD           dwBufferSize;
        LPWSTR          lpBuffer;
        DWORD           dwBufferUsed;
    };

}

#endif
