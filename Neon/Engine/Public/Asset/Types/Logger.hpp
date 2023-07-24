#pragma once

#include <Asset/Asset.hpp>

#include <map>
#include <Log/Logger.hpp>

namespace Neon::Asset
{
    class LoggerAsset : public IAsset
    {
    public:
        class Handler;

        using IAsset::IAsset;

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

    private:
        std::map<StringU8, Logger::LogSeverity> m_LogSeverityMap;
    };
} // namespace Neon::Asset