#include <CorePCH.hpp>
#include <Log/Logger.hpp>

#include <cassert>
#include <chrono>
#include <fstream>
#include <filesystem>

namespace Neon::Logger
{
    class LoggerImpl
    {
    public:
        LoggerImpl();
        ~LoggerImpl();

        void LogMessage(const StringU8& Message);

    private:
        std::ofstream m_LogFile;
        StringU8      m_FileName;
    };

    std::unique_ptr<LoggerImpl> s_LoggerImpl;

    void Initialize()
    {
        s_LoggerImpl = std::make_unique<LoggerImpl>();
    }

    void LogData(
        const StringU8& Message,
        LogSeverity     Severity,
        const char*     FunctionName,
        const char*     SourcePath,
        uint32_t        LineNum)
    {
        const char* Tag = "";
        switch (Severity)
        {
#if !NEON_DIST
        case Logger::LogSeverity::Info:
            Tag = "Info";
            break;
        case Logger::LogSeverity::Message:
            Tag = "Message";
            break;
        case Logger::LogSeverity::Warning:
            Tag = "Warning";
            break;
        case Logger::LogSeverity::Assert:
#endif
        case Logger::LogSeverity::Error:
            Tag = "Error";
            break;
        case Logger::LogSeverity::Fatal:
            Tag = "Fatal";
            break;
        }

#if !NEON_DIST
        StringU8 FormattedMsg =
            StringUtils::Format(
                "[{}]\t'{}': {}:({}) :: {}.\n",
                Tag,
                FunctionName,
                SourcePath,
                LineNum,
                Message);

        if (Severity == LogSeverity::Assert)
        {
#if NEON_RELEASE
            throw std::runtime_error(FormattedMsg);
#else
            auto wMessage    = StringUtils::StringTransform<String>(Message);
            auto wSourcePath = StringUtils::StringTransform<String>(SourcePath);

            Logger::Shutdown();
            _wassert(wMessage.c_str(), wSourcePath.c_str(), LineNum);
            Logger::Initialize();
#endif
        }
#else
        StringU8 FormattedMsg =
            StringUtils::Format(
                "[{}]\t :: {}.\n",
                Tag,
                Message);
#endif
        s_LoggerImpl->LogMessage(FormattedMsg);
    }

    void Shutdown()
    {
        s_LoggerImpl = nullptr;
    }

    LoggerImpl::LoggerImpl()
    {
        m_FileName = StringUtils::Format(
            "./Logs/Session_{0:%g}_{0:%h}_{0:%d} ({0:%H}-{0:%OM}-{0:%OS}).log",
            std::chrono::system_clock::now());

        if (!std::filesystem::exists("Logs"))
        {
            std::filesystem::create_directory("Logs");
        }
        m_LogFile.open(m_FileName);
        NEON_ASSERT(m_LogFile);
    }

    LoggerImpl::~LoggerImpl()
    {
        if (!m_LogFile.tellp())
        {
            m_LogFile.close();
            std::remove(m_FileName.c_str());
        }
    }

    void LoggerImpl::LogMessage(const StringU8& Message)
    {
        m_LogFile << Message;
        m_LogFile << std::flush;
    }
} // namespace Neon::Logger
