#include "winevtwrapper.h"

namespace wew {

    std::wstring WinEvtXmlWrapper::ExecuteQuery()
    {
        std::wstring xmlEvents{ L"<Events></Events>" };

        m_Errors.clear();

        try {
            m_hSession = OpenRemoteSession(m_computer.c_str());
            EvtHandleHolder evtSessionDeleter(&m_hSession);

            EVT_HANDLE hEvents = EvtQuery(m_hSession, 0,
                m_query.c_str(), m_queryFlags);
            if (hEvents == 0) {
                throw WinEvtXmlException{ GetLastError(), L"Query failed!" };
            }
            EvtHandleHolder evtEventsDeleter(&hEvents);

            xmlEvents.insert(8, FetchEvents(hEvents));
            xmlEvents.insert(0, L"<\?xml version=\"1.0\"\?>");
        }
        catch (const WinEvtXmlException& exception) {
            PushError(exception.lastErrorCode, exception.message);
            return {};
        }

        return xmlEvents;
    }

    EVT_HANDLE WinEvtXmlWrapper::OpenRemoteSession(LPCWSTR lpwszRemote)
    {
        EVT_RPC_LOGIN credentials{ const_cast<wchar_t*>(lpwszRemote) };
        EVT_HANDLE hRemote = EvtOpenSession(EvtRpcLogin, &credentials, 0, 0);

        SecureZeroMemory(&credentials, sizeof(EVT_RPC_LOGIN));

        if (hRemote == 0) {
            throw WinEvtXmlException{
                GetLastError(),
                L"Cannot open remote session!"
            };
        }

        return hRemote;
    }

    std::wstring WinEvtXmlWrapper::FetchEvents(EVT_HANDLE hEvents)
    {
        std::wstring xmlEvents;
        
        EVT_HANDLE	hEventsPack[EventsPackCount];
        DWORD		returned{ 0 };

        while (EvtNext(hEvents, EventsPackCount, hEventsPack, 
            INFINITE, 0, &returned)) {
            for (DWORD i = 0; i < returned; ++i) {
                std::wstring xmlEvent = RenderEventToXml(hEventsPack[i]);
                xmlEvents.append(xmlEvent);
                xmlEvents.append(L"\n");
            }
        }
        DWORD lastError = GetLastError();
        bool hasError = (lastError != ERROR_SUCCESS && 
            lastError != ERROR_NO_MORE_ITEMS);

        if (hasError) {
            throw WinEvtXmlException{
                GetLastError(),
                 L"Events fetching failed!"
            };
        }

        return xmlEvents;
    }

    std::wstring WinEvtXmlWrapper::RenderEventToXml(EVT_HANDLE hEvent)
    {
        std::wstring xmlEvent;

        EVT_HANDLE hContext = CreateProviderRenderContext();
        if (hContext == 0) {
            PushError(GetLastError(), L"Create render context failed!");
            return {};
        }
        EvtHandleHolder evtContextDeleter(&hContext);
        
        PEVT_VARIANT varPublisherName = static_cast<PEVT_VARIANT>(
            Render(hContext, hEvent, EvtRenderEventValues));
        LPCWSTR pwszPublisherName = varPublisherName[0].StringVal;

        LPCWSTR content {};
        EVT_HANDLE hProviderMetadata =
            EvtOpenPublisherMetadata(m_hSession, pwszPublisherName, 0, 0, 0);
        if (hProviderMetadata != 0) {
            content = FormatEventMessageString(hProviderMetadata, hEvent);
        }
        else {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_FILE_NOT_FOUND) {
                content = static_cast<LPCWSTR>(
                    Render(0, hEvent, EvtRenderEventXml));
            }
            else {
                PushError(lastError, L"Render event failed!");
                return {};
            }
        }
        EvtHandleHolder evtProviderMetadataDeleter(&hProviderMetadata);

        if (content != nullptr) {
            xmlEvent = content;
        }

        return xmlEvent;
    }

    EVT_HANDLE WinEvtXmlWrapper::CreateProviderRenderContext()
    {
        const DWORD valuesCount{ 1 };
        LPCWSTR		ppValues[]{ L"Event/System/Provider/@Name" };

        EVT_HANDLE hContext = EvtCreateRenderContext(
            valuesCount,
            ppValues,
            EvtRenderContextValues);

        return hContext;
    }

    LPVOID WinEvtXmlWrapper::Render(EVT_HANDLE hContext, EVT_HANDLE hEvent,
        EVT_RENDER_FLAGS flags)
    {
        EvtRenderCaller callEvtRender{ hContext, hEvent, flags };

        bool isRenderSucceed = callEvtRender();
        if (!isRenderSucceed) {
            DWORD lastError = GetLastError();
            bool isIsufficientBuffer{ lastError == ERROR_INSUFFICIENT_BUFFER };
            bool isSucceed{ lastError == ERROR_SUCCESS };

            if (isIsufficientBuffer) {
                callEvtRender.dwBufferSize = callEvtRender.dwBufferUsed;
                callEvtRender.pRenderedValues =
                    new LPVOID[callEvtRender.dwBufferUsed];

                if (callEvtRender.pRenderedValues != nullptr) {
                    callEvtRender();
                }
                else {
                    PushError(lastError, L"Render provider: Out of memory!");
                }
            }
            else if (!isIsufficientBuffer && !isSucceed) {
                PushError(lastError, L"Render provider failed!");
            }
        }

        if (callEvtRender.pRenderedValues != nullptr) {
            return callEvtRender.pRenderedValues;
        }

        return nullptr;
    }

    LPWSTR WinEvtXmlWrapper::FormatEventMessageString(
        EVT_HANDLE hMetadata,
        EVT_HANDLE hEvent) {
        
        EvtFormatMessageCaller callEvtFormatMessage{ hMetadata, hEvent };

        bool isFormatMessageSucceed = callEvtFormatMessage();
        if (!isFormatMessageSucceed) {
            DWORD lastError = GetLastError();
            bool isIsufficientBuffer{ lastError == ERROR_INSUFFICIENT_BUFFER };
            bool isSucceed{ lastError == ERROR_SUCCESS };

            if (isIsufficientBuffer)
            {
                callEvtFormatMessage.dwBufferSize = 
                    callEvtFormatMessage.dwBufferUsed;
                callEvtFormatMessage.lpBuffer = 
                    new wchar_t[callEvtFormatMessage.dwBufferSize];

                if (callEvtFormatMessage.lpBuffer) {
                    callEvtFormatMessage();
                }
                else {
                    PushError(lastError, L"Format message: Out of memory!");
                }
            }
            else if (!isIsufficientBuffer && !isSucceed) {
                PushError(lastError, L"Format message failed!");
            }
        }

        return callEvtFormatMessage.lpBuffer;
    }

    std::wstring LoadFile(std::wstring path) {
        const int MaxBufferSize = 1024;

        std::wstring content;

        std::wifstream file(path);
        wchar_t line[MaxBufferSize];
        if (file.is_open()) {
            while (!file.fail()) {
                file.getline(line, MaxBufferSize);
                content += line;
            }
            file.close();
        }

        return content;
    }

    bool WinEvtXmlWrapper::LoadQueryFromFile(const std::wstring& path)
    {
        m_query = LoadFile(path);

        if (m_query.length() == 0) {
            return false;
        }
        
        return true;
    }

    ErrorsVector WinEvtXmlWrapper::GetErrors() const
    {
        return m_Errors;
    }

    void WinEvtXmlWrapper::PushError(const DWORD lastErrorCode,
        const wchar_t* message) 
    {
        m_Errors.push_back({ lastErrorCode, message });
    }

    std::wstring WinEvtXmlWrapper::GetComputer() const
    {
        return m_computer;
    }

    void WinEvtXmlWrapper::SetComputer(const std::wstring& value)
    {
        if (m_computer != value) {
            m_computer = value;
        }
    }
    
    std::wstring WinEvtXmlWrapper::GetQuery() const
    {
        return m_query;
    }
    
    void WinEvtXmlWrapper::SetQuery(const std::wstring& value)
    {
        if (m_query != value) {
            m_query = value;
        }
    }
    
    DWORD WinEvtXmlWrapper::GetQueryFlags() const
    {
        return m_queryFlags;
    }
    
    void WinEvtXmlWrapper::SetQueryFlags(const DWORD value)
    {
        if (m_queryFlags != value) {
            m_queryFlags = value;
        }
    }

    bool EvtRenderCaller::operator()()
    {
        return EvtRender(
            hContext,
            hEvent,
            dwFlags,
            dwBufferSize,
            pRenderedValues,
            &dwBufferUsed,
            &dwPropertyCount);
    }

    bool EvtFormatMessageCaller::operator()()
    {
        return EvtFormatMessage(
            hMetadata,
            hEvent,
            dwMessageId,
            dwValuesCount,
            pRenderedValues,
            dwFlags,
            dwBufferSize,
            lpBuffer,
            &dwBufferUsed);
    }

}