#pragma once

#include <Core/String.hpp>
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
            ImTextureID TextureID;
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
                if (auto TextureID = Prop->get_optional<uint64_t>("texture"))
                {
                    Info.TextureID = reinterpret_cast<ImTextureID>(TextureID.get());
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
            else if (auto TextureID = m_Properties.get_optional<uint64_t>(Key))
            {
                Info.TextureID = reinterpret_cast<ImTextureID>(TextureID.get());
            }
            return Info;

            Info.TextureID = reinterpret_cast<ImTextureID>(Get<uint64_t>(Key + ".TextureID"));
            return Info;
        }

    private:
        boost::property_tree::ptree m_Properties;
    };
} // namespace Neon::Editor