#include <EnginePCH.hpp>
#include <Resource/Types/Logger.hpp>

namespace Neon::Asset
{
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

    size_t LoggerAsset::Handler::QuerySize(
        const Ptr<IAssetResource>& Resource)
    {
        size_t DataSize = 0;
        auto&  Tags     = static_cast<LoggerAsset*>(Resource.get())->GetTags();
        for (auto& Tag : Tags | std::views::keys)
        {
            DataSize += Tag.size() + 1;
            DataSize += sizeof(Logger::LogSeverity);
        }
        return DataSize;
    }

    Ptr<IAssetResource> LoggerAsset::Handler::Load(
        std::istream& Stream,
        size_t        DataSize)
    {
        auto   Asset(std::make_shared<LoggerAsset>());
        size_t Severity;
        for (size_t i = 0; i < DataSize; i++)
        {
            StringU8 Tag;
            Stream >> Tag >> Severity;
            Asset->m_LogSeverityMap[std::move(Tag)] = static_cast<Logger::LogSeverity>(Severity);
        }
        return Asset;
    }

    void LoggerAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        std::ostream&              Stream,
        size_t                     DataSize)
    {
        auto& Tags = static_cast<LoggerAsset*>(Resource.get())->GetTags();
        for (auto& [Tag, Severity] : Tags)
        {
            Stream << Tag << uint8_t(Severity);
        }
    }
} // namespace Neon::Asset