#include "Logger.h"
#include "PlatformDefines.h"
#include <ctime>
#include <sstream>
#include <cassert>
#include <iostream>
#include <limits>

#if YAGE_PLATFORM_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <debugapi.h>
#include <synchapi.h>
#include <profileapi.h>
#elif YAGE_PLATFORM_UNIX
#include <chrono>
#endif

#include <time.h>

class FileWriteTimer
{
public:
#if YAGE_PLATFORM_WINDOWS
    FileWriteTimer() : m_startTime(), m_endingTime(), m_frequency()
    {
        QueryPerformanceFrequency(&m_frequency);
        timeBeginPeriod(1);
        QueryPerformanceCounter(&m_startTime);
    }
    ~FileWriteTimer()
    {
        timeEndPeriod(1);
    }

    int64_t Query()
    {
        QueryPerformanceCounter(&m_endingTime);

        m_endingTime.QuadPart *= 1000000ll;
        m_endingTime.QuadPart /= m_frequency.QuadPart;
        return m_endingTime.QuadPart;
    }

private:
    LARGE_INTEGER m_startTime;
    LARGE_INTEGER m_endingTime;
    LARGE_INTEGER m_frequency;
#elif YAGE_PLATFORM_UNIX
    FileWriteTimer() 
    {
        m_startTime = std::chrono::high_resolution_clock::now();
    }
    
    ~FileWriteTimer() {}

    int64_t Query()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime).count();
    }

private:
    std::chrono::high_resolution_clock::time_point m_startTime;
#endif
};

unsigned int Logger::Logging_Helpers::UInt32ToStringInPlace(unsigned int value, char* buffer, unsigned int length)
{
    if (buffer == nullptr)
    {
        return 0;
    }

    unsigned int i = value;
    char* writePos = buffer + length - 1; // Start from the end of the array
    unsigned int generatedChars = 0;
    do {
        unsigned int prev = i;
        i /= 10;
        *writePos-- = '0' + (prev - i * 10);
        generatedChars++;
    } while (writePos >= buffer);
    return generatedChars;
}

void Logger::Logging_Helpers::GetFormatedDateTime(char* buffer)
{
    time_t now = std::time(0);
    std::tm ltm{};

#if YAGE_PLATFORM_WINDOWS
    localtime_s(&ltm, &now);
#elif YAGE_PLATFORM_UNIX
    // Linux uses different parameter order or gmtime_r/localtime_r instead
    localtime_r(&now, &ltm);
#endif

    buffer += UInt32ToStringInPlace(ltm.tm_year + 1900, buffer, 4);
    AppendNextDateTimeTemplateElement(buffer, 0);

    buffer += UInt32ToStringInPlace(ltm.tm_mon + 1, buffer, 2);
    AppendNextDateTimeTemplateElement(buffer, 1);

    buffer += UInt32ToStringInPlace(ltm.tm_mday, buffer, 2);
    AppendNextDateTimeTemplateElement(buffer, 2);

    buffer += UInt32ToStringInPlace(ltm.tm_hour, buffer, 2);
    AppendNextDateTimeTemplateElement(buffer, 3);

    buffer += UInt32ToStringInPlace(ltm.tm_min, buffer, 2);
    AppendNextDateTimeTemplateElement(buffer, 4);

    buffer += UInt32ToStringInPlace(ltm.tm_sec, buffer, 2);
    AppendNextDateTimeTemplateElement(buffer, 5);
}

void Logger::Logging_Helpers::FormatFileLine(char* buffer, unsigned int totalLength, unsigned int fileLength, const char* path, const int line)
{
    char* initialPos = buffer;
    memcpy(buffer, GetFileLineTemplate(0), ConstLen(GetFileLineTemplate(0)));
    buffer += 4;
    memcpy(buffer, path, fileLength);
    buffer += fileLength;
    memcpy(buffer, GetFileLineTemplate(1), ConstLen(GetFileLineTemplate(1)));
    buffer++;
    UInt32ToStringInPlace(line, buffer, GetDigits(line));
    memcpy(initialPos + totalLength - 3, GetFileLineTemplate(2), ConstLen(GetFileLineTemplate(2)));
}

void Logger::Logging_Helpers::AppendNextDateTimeTemplateElement(char*& buffer, unsigned int index)
{
    memcpy(buffer, GetDateTimeTemplate(index), ConstLen(GetDateTimeTemplate(index)));
    buffer += ConstLen(GetDateTimeTemplate(index));
}

#if YAGE_PLATFORM_WINDOWS
void Logger::Logging_Helpers::VSDebugOut(const char* message)
{
    OutputDebugStringA(message);
}
#elif YAGE_PLATFORM_UNIX
void Logger::Logging_Helpers::VSDebugOut(const char* message)
{
    // On Linux, redirect to console/stderr as there's no OutputDebugString equivalent
    fprintf(stderr, "%s", message);
}
#endif

void Logger::Logging_Helpers::ConsoleOut(const char* message)
{
    printf("%s", message);
}

//see https://stackoverflow.com/questions/1068849/how-do-i-determine-the-number-of-digits-of-an-integer-in-c
unsigned int Logger::Logging_Helpers::GetDigits(int value)
{
    if (value < 0) value = (value == std::numeric_limits<int>::min()) ? std::numeric_limits<int>::max() : -value;
    if (value > 999999999) return 10;
    if (value > 99999999) return 9;
    if (value > 9999999) return 8;
    if (value > 999999) return 7;
    if (value > 99999) return 6;
    if (value > 9999) return 5;
    if (value > 999) return 4;
    if (value > 99) return 3;
    if (value > 9) return 2;
    return 1;
}

std::unique_ptr<Logger::GlobalLogBuffer> Logger::GlobalLogBuffer::m_instance;
std::once_flag Logger::GlobalLogBuffer::m_onceFlag;

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

void Logger::FileOutput::Clear()
{
    if (!m_instance)
    {
        return;
    }
    m_instance.get()->m_writer.m_clear = true;
    volatile uint32_t waitloop = 0;
    while (m_instance.get()->m_writer.m_clear == true)
    {
        waitloop++; //busy loop waiting for the last log to be cleared
    }
}

Logger::FileOutput::FileOutput(const char* filePath)
{
    m_writer.m_fileHandle = std::make_unique<std::fstream>(std::fstream(filePath, std::ios::out | std::ios::trunc));
    m_writer.filePath = filePath;
    m_writerThread = std::thread(std::ref(m_writer));
}

void Logger::FileOutput::Writer::operator()()
{
    m_isRunning = true;
    FileWriteTimer timer;
    uint64_t previousTime = timer.Query();

    EzString message;
    while (m_isRunning)
    {
        if (m_buffer.PopIfPossible(message))
        {
            m_fileHandle.get()->write(message.c_str(), message.size());
            
            uint64_t now = timer.Query();
            uint64_t delta = now - previousTime;
            if (delta >= FILE_MESSAGE_FLUSH_TIME_MICRONS)
            {
                previousTime = now;
                m_fileHandle.get()->flush();
            }
        }
        if (m_clear)
        {
            m_fileHandle.get()->close();
            m_fileHandle.reset(new std::fstream(filePath, std::ios::out | std::ios::trunc));
            m_clear = false;
        }
    }
}

void Logger::GlobalLogBuffer::Add(const EzString& message, LogLevel level)
{
    if (!m_instance)
    {
        Init();
    }

    m_instance.get()->Push(message, level);
}

unsigned int Logger::GlobalLogBuffer::GetMessageCount()
{
    if (!m_instance)
    {
        return 0;
    }

    return m_instance.get()->m_addedMessages;
}

void Logger::GlobalLogBuffer::GetLogMessage(EzString& message, LogLevel& level, unsigned int index)
{
    if (!m_instance)
    {
        return;
    }

    m_instance.get()->PeekAtPosition(message, level, index);
}

unsigned int Logger::GlobalLogBuffer::GetCurrentMessageIndex()
{
    if (!m_instance)
    {
        return 0;
    }

    return m_instance.get()->m_writeIndex;
}

void Logger::GlobalLogBuffer::Clear()
{
    if (!m_instance)
    {
        return;
    }
}

void Logger::GlobalLogBuffer::Init()
{
    std::call_once(m_onceFlag,
        [] {
            m_instance.reset(new GlobalLogBuffer());
        });
}

Logger::GlobalLogBuffer::GlobalLogBuffer() : m_writeIndex(0), m_logLevelBuffer() {}

void Logger::GlobalLogBuffer::Push(const std::string& message, LogLevel level)
{
    if (m_addedMessages < BUFFER_SIZE) m_addedMessages++;

    unsigned int writeIndex = m_writeIndex.fetch_add(1) % BUFFER_SIZE;

    m_messageBuffer[writeIndex] = message;
    m_logLevelBuffer[writeIndex] = level;
}

void Logger::GlobalLogBuffer::PeekAtPosition(EzString& messageOut, LogLevel& level, unsigned int n)
{
    unsigned int readIndex = (m_writeIndex - 1 - n) % BUFFER_SIZE;
    messageOut = m_messageBuffer[readIndex];
    level = m_logLevelBuffer[readIndex];
}
