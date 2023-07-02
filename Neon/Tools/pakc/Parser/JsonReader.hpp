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

    /// <summary>
    /// Validates the fields of the JSON object.
    /// </summary>
    template<typename... _Args>
        requires(std::is_same_v<_Args, const char*> && ...)
    void ValidateFields(
        const boost::json::object& Object,
        _Args... Fields)
    {
        auto ParseOne = [&Object](const char* Field)
        {
            if (!Object.contains(Field))
            {
                throw std::runtime_error(Neon::StringUtils::Format("Failed to parse JSON: Missing field: '{}'", Field));
            }
        };

        (ParseOne(Fields), ...);
    }
} // namespace PakC