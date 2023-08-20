#pragma once

#include <Asset/Handle.hpp>
#include <Editor/Views/View.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ImGui/imgui.h>

namespace Neon::Editor
{
    class Profile
    {
    public:
        explicit Profile(
            const Asset::Handle& ProfileAsset);

        /// <summary>
        /// Get the value of the key or the default value if the key doesn't exist.
        /// </summary>
        template<typename _Ty>
        _Ty Get(
            const StringU8& Key,
            const _Ty&      DefaultValue = _Ty()) const
        {
            return m_Properties.get<_Ty>(Key, DefaultValue);
        }

        /// <summary>
        /// Get the value of the key or the default value if the key doesn't exist.
        /// </summary>
        ImTextureRectInfo GetTexture(
            const StringU8& Key) const
        {
            ImTextureRectInfo Info;
            if (auto AssetGuid = m_Properties.get_optional<StringU8>(Key))
            {
                Info.TextureID = LoadTexture(*AssetGuid);
            }
            else if (auto Prop = m_Properties.get_child_optional(Key))
            {
                if (auto AssetGuid = Prop->get_optional<StringU8>("texture"))
                {
                    Info.TextureID = LoadTexture(*AssetGuid);
                }
                auto ParseUV = [&](const boost::optional<StringU8>& UV, ImVec2& OutUV)
                {
                    if (!UV)
                    {
                        return;
                    }
                    auto Pos = UV->find(',');
                    if (Pos != StringU8::npos)
                    {
                        OutUV.x = std::stof(UV->substr(0, Pos));
                        OutUV.y = std::stof(UV->substr(Pos + 1));
                    }
                };
                ParseUV(Prop->get_optional<StringU8>("Min-uv"), Info.MinUV);
                ParseUV(Prop->get_optional<StringU8>("Max-uv"), Info.MaxUV);
            }
            return Info;
        }

    private:
        /// <summary>
        /// Load the texture from the path.
        /// </summary>
        [[nodiscard]] ImTextureID LoadTexture(
            const StringU8& AssetGuid) const;

    private:
        boost::property_tree::ptree m_Properties;

        mutable std::unordered_map<Asset::Handle, ImTextureID> m_LoadedTextures;
    };
} // namespace Neon::Editor