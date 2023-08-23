#include <EnginePCH.hpp>
#include <Editor/Project/DefaultFonts.hpp>
#include <UI/Fonts/FontAwesome5.hpp>
#include <ImGui/imgui.h>

//

extern unsigned int Arimo_Medium_compressed_data[];
extern unsigned int Arimo_Medium_compressed_size;

extern unsigned int FontAwesomeSolid900_compressed_data[];
extern unsigned int FontAwesomeSolid900_compressed_size;

//

namespace Neon::Editor
{
    void LoadDefaultFonts()
    {
        ImGuiIO& IO = ImGui::GetIO();

        // First is the interface's default font
        {
            ImFontConfig FontCfg;
            FontCfg.OversampleH = 2;

            IO.Fonts->AddFontFromMemoryCompressedTTF(
                Arimo_Medium_compressed_data,
                Arimo_Medium_compressed_size,
                15.f,
                &FontCfg,
                ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
        }

        // Second is the interface's default icons
        {
            ImFontConfig FontCfg;
            FontCfg.MergeMode = true;

            static const ImWchar IconRanges[] = {
                ICON_MIN_FA, ICON_MAX_FA,
                0
            };

            IO.Fonts->AddFontFromMemoryCompressedTTF(
                FontAwesomeSolid900_compressed_data,
                FontAwesomeSolid900_compressed_size,
                13.f,
                &FontCfg,
                IconRanges);
        }

        IO.Fonts->Build();
    }
} // namespace Neon::Editor