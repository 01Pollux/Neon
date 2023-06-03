#include <EnginePCH.hpp>
#include <Resource/Types/Logger.hpp>

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

    size_t LoggerAsset::Handler::QuerySize(
        const Ptr<IAssetResource>& Resource)
    {
        size_t DataSize = IO::BinaryStream::Size<uint8_t>();
        auto&  Tags     = static_cast<LoggerAsset*>(Resource.get())->GetTags();
        for (auto& Tag : Tags | std::views::keys)
        {
            DataSize += IO::BinaryStream::Size(Tag);
            DataSize += IO::BinaryStream::Size<Logger::LogSeverity>();
        }
        return DataSize;
    }

    Ptr<IAssetResource> LoggerAsset::Handler::Load(
        IO::BinaryStreamReader Stream,
        size_t                 DataSize)
    {
        auto    Asset(std::make_shared<LoggerAsset>());
        uint8_t Severity;
        uint8_t Count = Stream.Read<uint8_t>();
        for (uint8_t i = 0; i < Count; i++)
        {
            auto Tag = Stream.Read<StringU8>();
            Stream.Read(Severity);
            Asset->m_LogSeverityMap[std::move(Tag)] = static_cast<Logger::LogSeverity>(Severity);
        }
        return Asset;
    }

    void LoggerAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        IO::BinaryStreamWriter     Stream,
        size_t                     DataSize)
    {
        auto& Tags = static_cast<LoggerAsset*>(Resource.get())->GetTags();
        Stream.Write(uint8_t(Tags.size()));
        for (auto& [Tag, Severity] : Tags)
        {
            Stream.Write(Tag);
            Stream.Write(Severity);
        }
    }
} // namespace Neon::Asset