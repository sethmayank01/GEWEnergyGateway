#include "HttpUploader.h"

#include "../utils/Logger.h"

#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")


HttpUploader::HttpUploader(
    const std::string& url)
    :
    m_url(url)
{

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
            L"www.geworks.co.in",
            INTERNET_DEFAULT_HTTPS_PORT,
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
            L"/energy/api/upload.php",
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);



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



    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);



    return status == 200;
}