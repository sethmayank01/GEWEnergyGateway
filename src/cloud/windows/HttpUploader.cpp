#include "../HttpUploader.h"

#include "../../utils/Logger.h"

#include <windows.h>
#include <winhttp.h>
#include <vector>

#pragma comment(lib, "winhttp.lib")

HttpUploader::HttpUploader(
    const std::string& url)
    : m_url(url)
{
    std::wstring wurl(url.begin(), url.end());

    URL_COMPONENTS uc = {};

    uc.dwStructSize = sizeof(URL_COMPONENTS);

    uc.dwHostNameLength = static_cast<DWORD>(-1);
    uc.dwUrlPathLength = static_cast<DWORD>(-1);
    uc.dwExtraInfoLength = static_cast<DWORD>(-1);

    if (!WinHttpCrackUrl(
            wurl.c_str(),
            0,
            0,
            &uc))
    {
        Logger::Error("Invalid cloud URL.");

        return;
    }

    m_host.assign(
        uc.lpszHostName,
        uc.dwHostNameLength);

    m_path.assign(
        uc.lpszUrlPath,
        uc.dwUrlPathLength);

    if (uc.dwExtraInfoLength > 0)
    {
        m_path.append(
            uc.lpszExtraInfo,
            uc.dwExtraInfoLength);
    }

    m_port = uc.nPort;

    m_secure =
        (uc.nScheme == INTERNET_SCHEME_HTTPS);

  /*  Logger::Info(
        "HTTP Host : " +
        std::string(m_host.begin(), m_host.end()));

    Logger::Info(
        "HTTP Path : " +
        std::string(m_path.begin(), m_path.end()));*/
}


bool HttpUploader::Upload(
    const std::string& json)
{

    Logger::Info(
        "Uploading JSON to cloud");


    HINTERNET session =
        WinHttpOpen(
            L"GEW Energy Gateway",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            NULL,
            NULL,
            0);


    if(!session)
        return false;



    HINTERNET connect =
  WinHttpConnect(
    session,
    m_host.c_str(),
    m_port,
    0);



    if(!connect)
    {
        WinHttpCloseHandle(session);
        return false;
    }



    HINTERNET request =
WinHttpOpenRequest(
    connect,
    L"POST",
    m_path.c_str(),
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            m_secure ? WINHTTP_FLAG_SECURE : 0);



    if(!request)
        return false;



    std::wstring headers =
        L"Content-Type: application/json\r\n";



    BOOL sent =
        WinHttpSendRequest(
            request,
            headers.c_str(),
            static_cast<DWORD>(-1),
            (LPVOID)json.data(),
            (DWORD)json.size(),
            (DWORD)json.size(),
            0);



    if(!sent)
    {
        Logger::Error(
            "HTTP Send Failed");

        return false;
    }



    if(!WinHttpReceiveResponse(
            request,
            NULL))
    {
        Logger::Error(
            "HTTP Response Failed");

        return false;
    }



    DWORD status = 0;

    DWORD size = sizeof(status);


    WinHttpQueryHeaders(
        request,
        WINHTTP_QUERY_STATUS_CODE |
        WINHTTP_QUERY_FLAG_NUMBER,
        NULL,
        &status,
        &size,
        NULL);



    Logger::Info(
        "HTTP Status: "
        + std::to_string(status));

std::string responseBody;

DWORD bytesAvailable = 0;

while (WinHttpQueryDataAvailable(
           request,
           &bytesAvailable)
       && bytesAvailable > 0)
{
    std::vector<char> buffer(bytesAvailable);

    DWORD bytesRead = 0;

    if (WinHttpReadData(
            request,
            buffer.data(),
            bytesAvailable,
            &bytesRead))
    {
        responseBody.append(
            buffer.data(),
            bytesRead);
    }
}


if (!responseBody.empty())
{
    Logger::Info(
        "Server Response:");

    Logger::Info(
        responseBody);
}

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);



    return status == 200;
}