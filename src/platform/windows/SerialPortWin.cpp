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
    dcb.StopBits = ONESTOPBIT;

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
    }

    if (!SetCommState(m_handle, &dcb))
        return false;

    COMMTIMEOUTS timeouts{};

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.ReadTotalTimeoutMultiplier = 10;

    SetCommTimeouts(m_handle, &timeouts);

    Logger::Info("Serial Port Opened.");

    return true;
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

    return WriteFile(
        m_handle,
        data.data(),
        static_cast<DWORD>(data.size()),
        &written,
        nullptr);
}

bool SerialPortWin::Read(
    std::vector<uint8_t>& data,
    size_t bytesToRead,
    uint32_t)
{
    data.resize(bytesToRead);

    DWORD received = 0;

    BOOL ok = ReadFile(
        m_handle,
        data.data(),
        static_cast<DWORD>(bytesToRead),
        &received,
        nullptr);

    data.resize(received);

    return ok;
}