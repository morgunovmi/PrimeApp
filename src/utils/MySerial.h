#pragma once

#include <Windows.h>
#include <source_location>
#include <spdlog/spdlog.h>
#include <string>
#include <strsafe.h>

class MySerial
{
public:
    MySerial(const std::string& com_port, DWORD COM_BAUD_RATE)
        : m_connected(false)
    {
        Connect(com_port, COM_BAUD_RATE);
    }

    bool WriteSerialPort(const std::string& data)
    {
        DWORD bytes_sent = 0;

        PurgeComm(m_ioHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT |
                              PURGE_TXABORT);
        return WriteFile(m_ioHandle, (void*) data.c_str(), data.length(),
                       &bytes_sent, nullptr) != 0;
    }

    bool CloseSerialPort()
    {
        if (!m_connected) { return true; }

        PurgeComm(m_ioHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT |
                                      PURGE_TXABORT);
        FlushFileBuffers(m_ioHandle);
        if (CloseHandle(m_ioHandle) != 0)
        {
            m_connected = false;
            return true;
        }
        PrintLastError(
                (LPTSTR) std::source_location::current().function_name());
        return false;
    }

    [[nodiscard]] bool IsConnected() const { return m_connected; }

    ~MySerial() { CloseSerialPort(); }

    void Connect(const std::string& com_port, DWORD COM_BAUD_RATE)
    {
        CloseSerialPort();

        PurgeComm(m_ioHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT |
                                      PURGE_TXABORT);
        m_ioHandle = CreateFileA(static_cast<LPCSTR>(com_port.c_str()),
                                 GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (m_ioHandle == INVALID_HANDLE_VALUE)
        {
            PrintLastError((LPTSTR) "Connect");
            return;
        }
        DCB dcbSerialParams = {0};

        if (!GetCommState(m_ioHandle, &dcbSerialParams))
        {
            PrintLastError(
                    (LPTSTR) std::source_location::current().function_name());
        }
        else
        {
            dcbSerialParams.BaudRate = COM_BAUD_RATE;
            dcbSerialParams.ByteSize = 8;
            dcbSerialParams.StopBits = ONESTOPBIT;
            dcbSerialParams.Parity = NOPARITY;
            dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

            if (!SetCommState(m_ioHandle, &dcbSerialParams))
            {
                PrintLastError((LPTSTR) std::source_location::current()
                                       .function_name());
            }
            else
            {
                m_connected = true;
                PurgeComm(m_ioHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
                spdlog::info("Connected to port {}", com_port);
            }
        }
    }

private:
    void PrintLastError(LPTSTR lpszFunction)
    {
        LPVOID lpMsgBuf;
        LPVOID lpDisplayBuf;
        DWORD dw = GetLastError();

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                              FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR) &lpMsgBuf, 0, NULL);

        // Display the error message and exit the process

        lpDisplayBuf = (LPVOID) LocalAlloc(LMEM_ZEROINIT,
                                           (lstrlen((LPCTSTR) lpMsgBuf) +
                                            lstrlen((LPCTSTR) lpszFunction) +
                                            40) * sizeof(TCHAR));
        StringCchPrintf((LPTSTR) lpDisplayBuf,
                        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                        TEXT("%s failed with error %d: %s"), lpszFunction, dw,
                        lpMsgBuf);

        spdlog::error("{}", (LPTSTR) lpDisplayBuf);
    }

private:
    bool m_connected;
    HANDLE m_ioHandle;
};
