#include <EnginePCH.hpp>
#include <Resource/Types/Logger.hpp>
#include <IO/Archive.hpp>

#include <boost/serialization/map.hpp>

namespace Neon::Asset
{
    void LoggerAsset::SetGlobal()
    {
        for (auto& [Tag, Severity] : m_LogSeverityMap)
        {
            Logger::SetLogTag(Tag, Severity);
        }
    }

    void LoggerAsset::SetLogTag(
        const StringU8&     Tag,
        Logger::LogSeverity Severity)
    {
        m_LogSeverityMap[Tag] = Severity;
    }

    //

    bool LoggerAsset::Handler::CanCastTo(
        const Ptr<IAssetResource>& Resource)
    {
        return dynamic_cast<LoggerAsset*>(Resource.get());
    }

    Ptr<IAssetResource> LoggerAsset::Handler::Load(
        IO::InArchive& Archive,
        size_t)
    {
        auto Asset(std::make_shared<LoggerAsset>());
        Archive >> Asset->m_LogSeverityMap;
        return Asset;
    }

    void LoggerAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        IO::OutArchive&            Archive)
    {
        Archive << static_cast<LoggerAsset*>(Resource.get())->GetTags();
    }
} // namespace Neon::Asset