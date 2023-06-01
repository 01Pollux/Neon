#pragma once

#include <Resource/Pack.hpp>
#include <Resource/Manager.hpp>

#include <boost/json.hpp>
#include <functional>

namespace PakC
{
    using AssetResourcePtr = Neon::Ptr<Neon::Asset::IAssetResource>;
    using AssetPackPtr     = Neon::UPtr<Neon::Asset::IAssetPack>;

    class JsonHandler
    {
    public:
        JsonHandler();

        NEON_CLASS_NO_COPYMOVE(JsonHandler);

        ~JsonHandler();

        /// <summary>
        /// Parses a JSON file.
        /// </summary>
        void Parse(
            const std::string& Path);

    private:
        /// <summary>
        /// Registers all the types that can be parsed.
        /// </summary>
        void RegisterTypes();

    private:
        Neon::UPtr<Neon::Asset::IResourceManager> m_ResourceManager;

        std::map<std::string, std::function<AssetResourcePtr(const boost::json::object&)>>
            m_AssetResources;

        std::vector<std::jthread> m_Threads;
    };
} // namespace PakC