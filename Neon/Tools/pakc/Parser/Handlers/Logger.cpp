#include <PakCPCH.hpp>
#include <Parser/Handlers/Logger.hpp>
#include <fstream>

#include <Resource/Types/Logger.hpp>
#include <Log/Logger.hpp>

namespace PakC::Handler
{
    using namespace Neon;
    AssetResourcePtr LoadLoggerResource(
        const boost::json::object& Object)
    {
        auto Asset = std::make_shared<Asset::LoggerAsset>();
        if (auto Tags = Object.if_contains("Tags"); Tags && Tags->is_object())
        {
            for (auto& [Tag, SeverityName] : Tags->as_object())
            {
                Logger::LogSeverity Severity;
                switch (StringUtils::Hash(StringU8(SeverityName.as_string())))
                {
                case StringUtils::Hash("Disabled"):
                    Severity = Logger::LogSeverity::Disabled;
                    break;
                case StringUtils::Hash("Trace"):
                    Severity = Logger::LogSeverity::Trace;
                    break;
                case StringUtils::Hash("Info"):
                    Severity = Logger::LogSeverity::Info;
                    break;
                case StringUtils::Hash("Warning"):
                    Severity = Logger::LogSeverity::Warning;
                    break;
                case StringUtils::Hash("Error"):
                    Severity = Logger::LogSeverity::Error;
                    break;
                case StringUtils::Hash("Fatal"):
                    Severity = Logger::LogSeverity::Fatal;
                    break;
                default:
                    throw std::runtime_error("Invalid log severity " + StringU8(SeverityName.as_string()));
                }
                Asset->SetLogTag(Tag, Severity);
            }
        }
        return Asset;
    }
} // namespace PakC::Handler