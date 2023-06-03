#pragma once

#include <Resource/Asset.hpp>
#include <Resource/Handler.hpp>

#include <Log/Logger.hpp>

namespace Neon::Asset
{
    class LoggerAsset : public IAssetResource
    {
        friend class Handler;

    public:
        /// <summary>
        /// Set log tags globally.
        /// </summary>
        void SetGlobal();

        /// <summary>
        /// Set the log tag to the specified severity.
        /// </summary>
        void SetLogTag(
            const StringU8&     Tag,
            Logger::LogSeverity Severity);

        /// <summary>
        /// Get the log tags.
        /// </summary>
        auto& GetTags() const noexcept
        {
            return m_LogSeverityMap;
        }

    public:
        class Handler : public IAssetResourceHandler
        {
        public:
            bool CanCastTo(
                const Ptr<IAssetResource>& Resource) override;

            size_t QuerySize(
                const Ptr<IAssetResource>& Resource) override;

            Ptr<IAssetResource> Load(
                IO::BinaryStreamReader Stream,
                size_t                 DataSize) override;

            void Save(
                const Ptr<IAssetResource>& Resource,
                IO::BinaryStreamWriter     Stream,
                size_t                     DataSize) override;
        };

    private:
        std::map<StringU8, Logger::LogSeverity> m_LogSeverityMap;
    };
} // namespace Neon::Asset