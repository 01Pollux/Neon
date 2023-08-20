#pragma once

#include <Asset/Handle.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ImGui/imgui.h>

namespace Neon::Editor
{
    class Profile
    {
    public:
        Profile(
            const StringU8& Path = "");

        struct TextureRectInfo
        {
            ImTextureID TextureID{};
            ImVec2      MinUV = ImVec2(0.0f, 0.0f);
            ImVec2      MaxUV = ImVec2(1.0f, 1.0f);
        };

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
        TextureRectInfo GetTexture(
            const StringU8& Key) const
        {
            TextureRectInfo Info;
            if (auto Prop = m_Properties.get_child_optional(Key))
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
            else if (auto AssetGuid = m_Properties.get_optional<StringU8>(Key))
            {
                Info.TextureID = LoadTexture(*AssetGuid);
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