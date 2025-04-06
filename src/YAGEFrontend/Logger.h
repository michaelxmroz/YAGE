#pragma once
#include <memory>
#include <string>
#include <stdexcept>
#include <fstream>
#include <mutex>
#include <cassert>

//This can be your custom string type, as long as it supports .c_str()
#define EzString std::string

//set to 0 if you do not want to include windows.h for the visual studio debug out, or console out
#define VS_OUT 1
//set to 1 for windows high-precision timer
#define WIN_TIMER 1

#define DEFAULT_FILE_NAME "log.txt"
#define FILE_MESSAGE_BUFFER_SIZE 8000
#define FILE_MESSAGE_FLUSH_TIME_MICRONS 1000

namespace Logger
{
    namespace Logging_Helpers
    {
        constexpr unsigned int ConstLen(const char* str)
        {
            return *str ? 1 + ConstLen(str + 1) : 0;
        } 
        constexpr const char* GetDateTimeTemplate(unsigned int index)
        {
            const char* templateParts[6] = { "/", "/", " ", ":", ":", " - " };
            return templateParts[index];
        }
        constexpr const char* GetFileLineTemplate(unsigned int index)
        {
            const char* templateParts[3] = { " - [", "(", ")]\n" };
            return templateParts[index];
        }
        constexpr unsigned int GetFormatedDateTimeLength()
        {
            //TODO no magic constant
            return 20;
        }
        constexpr unsigned int GetFileLineTemplateLength()
        {
            return ConstLen(GetFileLineTemplate(0)) + ConstLen(GetFileLineTemplate(1)) + ConstLen(GetFileLineTemplate(2));
        }

        unsigned int UInt32ToStringInPlace(unsigned int value, char* buffer, unsigned int length);
        void GetFormatedDateTime(char* buffer);
        void FormatFileLine(char* buffer, unsigned int totalLength, unsigned int fileLength, const char* path, const int line);    
        void AppendNextDateTimeTemplateElement(char*& buffer, unsigned int index);

        void VSDebugOut(const char* message);
        void ConsoleOut(const char* message);
        unsigned int GetDigits(int value);

        // see https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
#pragma warning( push )
#pragma warning(disable : 4996)
        template<typename ... Args>
        std::string string_format(const EzString& format, Args&& ... args)
        {
            int size_s = std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args) ...) + 1; // Extra space for '\0'
            if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
            auto size = static_cast<size_t>(size_s);

            auto buf = std::make_unique<char[]>(size);
            std::snprintf(buf.get(), size, format.c_str(), args ...);
            return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
        }
#pragma warning( pop )

        //Ringbuffer for multi-threaded writing and single-threaded reading
        template <int ARRAY_SIZE>
        class LocklessRingBuffer
        {
        public:
            LocklessRingBuffer()
            {
                for (unsigned int i = 0; i < ARRAY_SIZE; ++i)
                {
                    m_readable[i].store(false);
                }
                m_writeIndex.store(0);
                m_readIndex = 0;
            }
            void Push(const EzString& message)
            {
                unsigned int writeIndex = m_writeIndex.fetch_add(1) % ARRAY_SIZE;

                m_buffer[writeIndex] = message;
                bool oldVal = false;
                while (m_readable[writeIndex].load())
                {
                    //busy loop
                }
                m_buffer[writeIndex] = message;
                m_readable[writeIndex].store(true);
            }
            bool PopIfPossible(EzString& messageOut)
            {
                unsigned int readIndex = m_readIndex % ARRAY_SIZE;
                if (!m_readable[readIndex])
                {
                    return false;
                }
                messageOut = m_buffer[readIndex];
                m_readIndex++;
                m_readable[readIndex].store(false);
                return true;
            }

            void Peek(EzString& messageOut)
            {
				unsigned int readIndex = m_readIndex % ARRAY_SIZE;
                if (!m_readable[readIndex])
                {
					return;
				}
				messageOut = m_buffer[readIndex];
			}

            unsigned int GetCurrentMessageIndex()
            {
				return m_writeIndex;
			}

        private:
            std::atomic<bool> m_readable[ARRAY_SIZE];
            EzString m_buffer[ARRAY_SIZE];

            std::atomic<unsigned int> m_writeIndex;
            unsigned int m_readIndex;
        };
    }

//Enums
//----------------------------------//
    enum class LogLevel : unsigned int
    {
        Info = 1,
        Warning = 2,
        Error = 4
    };

    constexpr LogLevel operator|(LogLevel a, LogLevel b)
    {
        return static_cast<LogLevel>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    }

    enum class LogVerbosityOptions : unsigned int
    {
        Level = 1,
        DateTime = 2,
        FileLine = 4,
    };

    constexpr LogVerbosityOptions operator|(LogVerbosityOptions a, LogVerbosityOptions b)
    {
        return static_cast<LogVerbosityOptions>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    }

    enum class LogSeverity : unsigned int
    {
        None = 0,
        Errors = static_cast<unsigned int>(LogLevel::Error),
        Warnings = static_cast<unsigned int>(LogLevel::Warning) | static_cast<unsigned int>(LogLevel::Error),
        All = static_cast<unsigned int>(LogLevel::Info) | static_cast<unsigned int>(LogLevel::Warning) | static_cast<unsigned int>(LogLevel::Error),
    };

    enum class LogVerbosity : unsigned int
    {
        Minimal = 0,
        Level = static_cast<unsigned int>(LogVerbosityOptions::Level),
        DateTime = static_cast<unsigned int>(LogVerbosityOptions::DateTime),
        FileLine = static_cast<unsigned int>(LogVerbosityOptions::FileLine),
        All = static_cast<unsigned int>(LogVerbosityOptions::Level) | static_cast<unsigned int>(LogVerbosityOptions::DateTime) | static_cast<unsigned int>(LogVerbosityOptions::FileLine),
    };

    template<LogLevel level>
    constexpr const char* LogLevelToString()
    {
        switch (level)
        {
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    template<LogLevel level>
    constexpr unsigned int LogLevelStringSize()
    {
        return Logging_Helpers::ConstLen(LogLevelToString<level>());
    }


    class GlobalLogBuffer
    {
    public:

        static void Add(const EzString& message, LogLevel level);;

        static unsigned int GetMessageCount();

        static void GetLogMessage(EzString& message, LogLevel& level, unsigned int index);

        static unsigned int GetCurrentMessageIndex();

        static void Clear();

    private:
        static void Init();

        static std::unique_ptr<GlobalLogBuffer> m_instance;
        static std::once_flag m_onceFlag;

        GlobalLogBuffer();

        void Push(const std::string& message, LogLevel level);

        void PeekAtPosition(EzString& messageOut, LogLevel& level, unsigned int n);

        static constexpr int BUFFER_SIZE = 1024;
        EzString m_messageBuffer[BUFFER_SIZE];
        LogLevel m_logLevelBuffer[BUFFER_SIZE];
        unsigned int m_addedMessages = 0;
        std::atomic<unsigned int> m_writeIndex;
    };


//Output Controllers
//----------------------------------//
#if VS_OUT
    class VSDebugOutput
    {
    public:
        template<LogLevel level>
        static void Output(const EzString& message)
        {
            Logging_Helpers::VSDebugOut(message.c_str());
        };
    };
#endif

    class ConsoleOutput
    {
    public:
        template<LogLevel level>
        static void Output(const EzString& message)
        {
            Logging_Helpers::ConsoleOut(message.c_str());
        };
    };

    class LogBufferOutput
    {
    public:
        template<LogLevel level>
        static void Output(const EzString& message)
        {
            GlobalLogBuffer::Add(message, level);
        };
    };

    //Singleton thread-safe file writer
    class FileOutput
    {
    public:
        ~FileOutput();

        static void Init(const char* filePath);

        template<LogLevel level>
        static void Output(const EzString& message)
        {
            if (!m_instance)
            {
                Init(DEFAULT_FILE_NAME);
            }

            m_instance.get()->m_writer.m_buffer.Push(message);
        };

        static void Clear();

    private:

        FileOutput(const char* filePath);

        FileOutput& GetInstance()
        {
            return *m_instance.get();
        }

        class Writer
        {
        public:
            Writer() {}
            void operator()();
            std::unique_ptr<std::fstream> m_fileHandle;
            Logging_Helpers::LocklessRingBuffer<FILE_MESSAGE_BUFFER_SIZE> m_buffer;
            bool m_isRunning = true;
            bool m_clear = false;
            EzString filePath;
        };


        Writer m_writer;
        std::thread m_writerThread;

        static std::unique_ptr<FileOutput> m_instance;
        static std::once_flag m_onceFlag;
    };

//Logging Boilerplate
//----------------------------------//
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable unreachable code warning caused by the constexpr ifs
    template <LogSeverity severity, LogVerbosity verbosity, class output>
    class SubLogger
    {
    public:
        template <LogLevel level>
        static void Log(const char* message, const char* path, const int line)
        {
            if constexpr ((static_cast<unsigned int>(level) & static_cast<unsigned int>(severity)) == 0)
            {
                return;
            }

            unsigned int prefixLength = 0;
            unsigned int messageLength = static_cast<unsigned int>(strlen(message));
            unsigned int postfixLength = 0;
            unsigned int pathLength = 0;

            GetTotalMessageLength<level>(prefixLength, postfixLength, pathLength, path, line);

            unsigned int totalLength = prefixLength + messageLength + postfixLength;
            EzString finalMessageBuffer;
            finalMessageBuffer.resize(totalLength);

            ComposeMessage<level>(finalMessageBuffer, prefixLength, message, messageLength, postfixLength, pathLength, path, line);

            output::Output<level>(finalMessageBuffer);
        }

        template<LogLevel level>
        static void ComposeMessage(std::string& finalMessageBuffer, unsigned int prefixLength, const char* message, unsigned int messageLength, unsigned int postfixLength, unsigned int pathLength, const char* path, int line)
        {
            char* writePosition = &finalMessageBuffer[0];

            if constexpr ((static_cast<unsigned int>(verbosity) & static_cast<unsigned int>(LogVerbosityOptions::DateTime)) != 0)
            {
                Logging_Helpers::GetFormatedDateTime(writePosition);
                writePosition += Logging_Helpers::GetFormatedDateTimeLength();
            }

            if constexpr ((static_cast<unsigned int>(verbosity) & static_cast<unsigned int>(LogVerbosityOptions::Level)) != 0)
            {
                unsigned int length = LogLevelStringSize<level>();
                memcpy(writePosition, LogLevelToString<level>(), length);
                writePosition += length;
            }

            if (prefixLength != 0)
            {
                memcpy(writePosition, ": ", 2);
            }

            writePosition = &finalMessageBuffer[0] + prefixLength;
            memcpy(writePosition, message, messageLength);
            writePosition += messageLength;

            if constexpr ((static_cast<unsigned int>(verbosity) & static_cast<unsigned int>(LogVerbosityOptions::FileLine)) != 0)
            {
                Logging_Helpers::FormatFileLine(writePosition, postfixLength, pathLength, path, line);
                writePosition += postfixLength;
            }
        }

        template<LogLevel level>
        static void GetTotalMessageLength(unsigned int& prefixLength, unsigned int& postfixLength, unsigned int& pathLength, const char* path, int line)
        {
            if constexpr ((static_cast<unsigned int>(verbosity) & static_cast<unsigned int>(LogVerbosityOptions::DateTime)) != 0)
            {
                prefixLength += Logging_Helpers::GetFormatedDateTimeLength();
            }

            if constexpr ((static_cast<unsigned int>(verbosity) & static_cast<unsigned int>(LogVerbosityOptions::Level)) != 0)
            {
                prefixLength += LogLevelStringSize<level>();
            }

            if (prefixLength != 0)
            {
                prefixLength += 2;
            }

            if constexpr ((static_cast<unsigned int>(verbosity) & static_cast<unsigned int>(LogVerbosityOptions::FileLine)) != 0)
            {
                pathLength = static_cast<unsigned int>(strlen(path));
                postfixLength += pathLength;
                postfixLength += Logging_Helpers::GetDigits(line);
                postfixLength += Logging_Helpers::GetFileLineTemplateLength();
            }
        }
    };
#pragma warning( pop )

//Compound loggers through template magic
//----------------------------------//
    template<LogLevel level, class T>
    void ResolveVariadicLogCall(const char* message, const char* path, const int line)
    {
        T::Log<level>(message, path, line);
    }

    template<LogLevel level, class T, class ... loggers>
    typename std::enable_if<sizeof ... (loggers) != 0, void>::type
        ResolveVariadicLogCall(const char* message, const char* path, const int line)
    {
        T::Log<level>(message, path, line);
        ResolveVariadicLogCall<level, loggers ... >(message, path, line);
    }

    template<class ... loggers>
    class CompoundLogger
    {
    public:
        template <LogLevel level>
        static void Log(const char* message, const char* path, const int line)
        {
            ResolveVariadicLogCall<level, loggers ... >(message, path, line);
        }
    };

/*
 In this section various loggers can be defined and compounded. A compound logger can contain any amount of loggers as template arguments.
 A different DefaultLogger could be defined for different build configs.
 The DefaultLogger is used by the default LOG_ macros.
 */
 //----------------------------------//
    typedef SubLogger<LogSeverity::All, LogVerbosity::All, VSDebugOutput> VSLogger;
    typedef SubLogger<LogSeverity::All, LogVerbosity::All, ConsoleOutput> ConsoleLogger;
    typedef SubLogger<LogSeverity::Errors, LogVerbosity::All, FileOutput> FileLogger;
    typedef SubLogger<LogSeverity::All, LogVerbosity::All, LogBufferOutput> GlobalBufferLogger;

    typedef SubLogger<LogSeverity::All, LogVerbosity::Minimal, FileOutput> FileLoggerMinimal;

    typedef CompoundLogger<GlobalBufferLogger, VSLogger> DefaultLogger;
    typedef CompoundLogger<FileLoggerMinimal> MinimalLogger;
}

#define LOG_INFO(message) \
{ \
 Logger::DefaultLogger::Log<Logger::LogLevel::Info>(message, __FILE__, __LINE__); \
}

#define LOG_WARNING(message) \
{ \
 Logger::DefaultLogger::Log<Logger::LogLevel::Warning>(message, __FILE__, __LINE__); \
}

#define LOG_ERROR(message) \
{ \
 Logger::DefaultLogger::Log<Logger::LogLevel::Error>(message, __FILE__, __LINE__); \
}

#define LOG_MINIMAL(message) \
{ \
Logger::MinimalLogger::Log<Logger::LogLevel::Info>(message, __FILE__, __LINE__); \
}

