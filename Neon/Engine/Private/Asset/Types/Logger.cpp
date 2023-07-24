#include <EnginePCH.hpp>
#include <Asset/Handlers/Logger.hpp>

#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

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
        MarkDirty();
        m_LogSeverityMap[Tag] = Severity;
    }

    //

    bool LoggerAsset::Handler::CanHandle(
        const Ptr<IAsset>& Asset)
    {
        return dynamic_cast<LoggerAsset*>(Asset.get());
    }

    Ptr<IAsset> LoggerAsset::Handler::Load(
        std::istream& Stream,
        const Asset::DependencyReader&,
        const Handle&        AssetGuid,
        StringU8             Path,
        const AssetMetaData& LoaderData)
    {
        auto Asset = std::make_shared<LoggerAsset>(AssetGuid, std::move(Path));

        boost::archive::text_iarchive Archive(Stream, boost::archive::no_header | boost::archive::no_tracking);
        Archive >> Asset->m_LogSeverityMap;

        return Asset;
    }

    void LoggerAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto LoggerAsset = static_cast<Asset::LoggerAsset*>(Asset.get());

        boost::archive::text_oarchive Archive(Stream, boost::archive::no_header | boost::archive::no_tracking);
        Archive << LoggerAsset->m_LogSeverityMap;
    }
} // namespace Neon::Asset