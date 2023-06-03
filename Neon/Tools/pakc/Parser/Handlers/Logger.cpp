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
        auto Tags  = Object.find("Tags");
        if (Tags != Object.end() && Tags->value().is_object())
        {
            for (auto& [Tag, SeverityName] : Tags->value().as_object())
            {
                Logger::LogSeverity Severity;
                switch (StringUtils::Hash(std::string(SeverityName.as_string())))
                {
                case StringUtils::Hash("Disabled"):
                    Severity = Logger::LogSeverity::Disabled;
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
                    Severity = Logger::LogSeverity::Disabled;
                    break;
                }
                Asset->SetLogTag(Tag, Severity);
            }
        }
        return Asset;
    }
} // namespace PakC::Handler