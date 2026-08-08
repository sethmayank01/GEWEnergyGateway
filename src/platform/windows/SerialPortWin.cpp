#include "SerialPortWin.h"

#include "utils/Logger.h"

SerialPortWin::SerialPortWin(const GatewayConfig::Meter& cfg)
    : m_handle(INVALID_HANDLE_VALUE),
      m_cfg(cfg)
{
}

SerialPortWin::~SerialPortWin()
{
    Close();
}

bool SerialPortWin::Open()
{
    std::string port = "\\\\.\\" + m_cfg.port;

    m_handle = CreateFileA(
        port.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        Logger::Error("Unable to open serial port.");
        return false;
    }

    DCB dcb{};
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(m_handle, &dcb))
        return false;

    dcb.BaudRate = m_cfg.baud;
dcb.ByteSize = 8;
dcb.StopBits = (m_cfg.stopBits == 2) ? TWOSTOPBITS : ONESTOPBIT;

switch (m_cfg.parity)
{
case 'E':
    dcb.Parity = EVENPARITY;
    break;

case 'O':
    dcb.Parity = ODDPARITY;
    break;

default:
    dcb.Parity = NOPARITY;
    break;
}

dcb.fBinary = TRUE;
dcb.fParity = (dcb.Parity != NOPARITY);
dcb.fOutxCtsFlow = FALSE;
dcb.fOutxDsrFlow = FALSE;
dcb.fDtrControl = DTR_CONTROL_DISABLE;
dcb.fRtsControl = RTS_CONTROL_DISABLE;
dcb.fOutX = FALSE;
dcb.fInX = FALSE;

    if (!SetCommState(m_handle, &dcb))
        return false;

    COMMTIMEOUTS timeouts{};

timeouts.ReadIntervalTimeout = 20;
timeouts.ReadTotalTimeoutMultiplier = 0;
timeouts.ReadTotalTimeoutConstant = 100;

timeouts.WriteTotalTimeoutMultiplier = 0;
timeouts.WriteTotalTimeoutConstant = 100;

if (!SetCommTimeouts(m_handle, &timeouts))
{
    Logger::Error("Failed to configure COM timeouts.");
    return false;
}
    Logger::Info("Serial Port Opened.");

    return true;
}

void SerialPortWin::Flush()
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        PurgeComm(
            m_handle,
            PURGE_RXCLEAR
            );
    }
}

void SerialPortWin::Close()
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

bool SerialPortWin::IsOpen() const
{
    return m_handle != INVALID_HANDLE_VALUE;
}

bool SerialPortWin::Write(const std::vector<uint8_t>& data)
{
    DWORD written = 0;

BOOL ok = WriteFile(
    m_handle,
    data.data(),
    static_cast<DWORD>(data.size()),
    &written,
    nullptr);

if (!ok)
{
    Logger::Error("WriteFile failed.");
    return false;
}

Logger::Info("Bytes Written: " + std::to_string(written));

return written == data.size();
}

bool SerialPortWin::Read(
    std::vector<uint8_t>& data,
    size_t bytesToRead,
    uint32_t timeoutMs)
{
    data.clear();
    data.resize(bytesToRead);

    DWORD totalReceived = 0;
    DWORD startTime = GetTickCount();

    while (totalReceived < bytesToRead)
    {
        DWORD received = 0;

        BOOL ok = ReadFile(
            m_handle,
            data.data() + totalReceived,
            static_cast<DWORD>(bytesToRead - totalReceived),
            &received,
            nullptr);

        if (!ok)
        {
            data.clear();
            return false;
        }

        if (received > 0)
        {
            totalReceived += received;

            Logger::Info(
                "Received " +
                std::to_string(received) +
                " byte(s), Total = " +
                std::to_string(totalReceived));
        }

        if (GetTickCount() - startTime >= timeoutMs)
        {
            data.resize(totalReceived);

            Logger::Warning(
                "Serial read timeout after receiving " +
                std::to_string(totalReceived) +
                " byte(s)");

            return false;
        }

        if (received == 0)
        {
            Sleep(2);
        }
    }

    data.resize(totalReceived);

    return true;
}
