#include "Logger.h"
#include <ctime>
#include <sstream>
#include <cassert>


#if VS_OUT
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#undef WIN32_LEAN_AND_MEAN
#include "debugapi.h"
#endif // VS_Out

#include <iostream>

void Logger::Logging_Helpers::MergeFinalMessage(const EzString& prefix, const EzString& message, const EzString& postfix, EzString& finalMessageOut)
{
    finalMessageOut = string_format("%s%s%s", prefix.c_str(), message.c_str(), postfix.c_str());
}

void Logger::Logging_Helpers::GetFormatedDateTime(EzString& dateTimeOut)
{
    time_t now = std::time(0);
    std::tm ltm{};
    localtime_s(&ltm, &now);
    std::stringstream time;
    time << ltm.tm_year + 1900 << "/" << ltm.tm_mon + 1 << "/" << ltm.tm_mday << " " << ltm.tm_hour << ":" << ltm.tm_min <<
        ":" << ltm.tm_sec << " - ";
    dateTimeOut = time.str().c_str();
}

Logger::Logging_Helpers::LocklessRingBuffer::LocklessRingBuffer()
{
    for (unsigned int i = 0; i < FILE_MESSAGE_BUFFER_SIZE; ++i)
    {
        m_readable[i].store(false);
    }
    m_writeIndex.store(0);
    m_readIndex = 0;
}

void Logger::Logging_Helpers::LocklessRingBuffer::Push(const EzString& message)
{
    unsigned int writeIndex = m_writeIndex.fetch_add(1) % FILE_MESSAGE_BUFFER_SIZE;

    m_buffer[writeIndex] = message;
    bool oldVal = false;
    if (!m_readable[writeIndex].compare_exchange_strong(oldVal, true))
    {
        assert(((void)"File message buffer overflow in logging system. Consider increasing buffer size", false));
    }
}

bool Logger::Logging_Helpers::LocklessRingBuffer::PopIfPossible(EzString& messageOut)
{
    unsigned int readIndex = m_readIndex % FILE_MESSAGE_BUFFER_SIZE;
    if (!m_readable[readIndex])
    {
        return false;
    }
    messageOut = m_buffer[readIndex];
    m_readIndex++;
    m_readable[readIndex].store(false);
    return true;
}

#if VS_OUT
void Logger::Logging_Helpers::VSDebugOut(const char* message)
{
    OutputDebugStringA(message);
}
#endif

void Logger::Logging_Helpers::ConsoleOut(const char* message)
{
    std::cout << message;
}

std::unique_ptr<Logger::FileOutput> Logger::FileOutput::m_instance;
std::once_flag Logger::FileOutput::m_onceFlag;

void Logger::FileOutput::Init(const char* filePath)
{
    std::call_once(m_onceFlag,
        [filePath] {
            m_instance.reset(new FileOutput(filePath));
        });
}

Logger::FileOutput::~FileOutput()
{
    m_writer.m_isRunning = false;
    m_writerThread.join();
    m_writer.m_fileHandle.get()->close();
}

Logger::FileOutput::FileOutput(const char* filePath)
{
    m_writer.m_fileHandle = std::make_unique<std::fstream>(std::fstream(filePath, std::ios::out | std::ios::ate));
    m_writerThread = std::thread(std::ref(m_writer));
}

void Logger::FileOutput::Writer::operator()()
{
    m_isRunning = true;

    while (m_isRunning)
    {
        EzString message;
        if (m_buffer.PopIfPossible(message))
        {
            m_fileHandle.get()->write(message.c_str(), message.size());
            m_fileHandle.get()->flush();
        }
    }
}