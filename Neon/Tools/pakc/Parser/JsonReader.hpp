#pragma once

#include <Core/String.hpp>

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
        using ResourceLoaderMap = std::map<Neon::StringU8, std::function<AssetResourcePtr(const boost::json::object&)>>;

    public:
        JsonHandler();

        /// <summary>
        /// Parses a JSON file.
        /// </summary>
        void Parse(
            const Neon::StringU8& Path);

    private:
        /// <summary>
        /// Registers all the types that can be parsed.
        /// </summary>
        void RegisterTypes();

    private:
        Neon::UPtr<Neon::Asset::IResourceManager> m_ResourceManager;
        ResourceLoaderMap                         m_AssetResources;
    };
} // namespace PakC