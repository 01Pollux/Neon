#pragma once

#include <Asset/Asset.hpp>

#include <boost/json.hpp>

namespace Neon::Asset
{
    class JsonAsset : public IAsset
    {
    public:
        class Handler;

        using IAsset::IAsset;

        /// <summary>
        /// Set json object.
        /// </summary>
        void SetJson(
            boost::json::value Json);

        /// <summary>
        /// Get json object.
        /// </summary>
        auto& GetJson() const noexcept
        {
            return m_Json;
        }

    private:
        boost::json::value m_Json;
    };
} // namespace Neon::Asset