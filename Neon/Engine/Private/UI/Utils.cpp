#include <EnginePCH.hpp>
#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::UI::Utils
{
    void DrawTextBG(
        const char* Text,
        const char* TextEnd)
    {
        auto   DrawList  = ImGui::GetWindowDrawList();
        ImVec2 Pos       = ImGui::GetCursorScreenPos();
        ImVec2 Size      = ImGui::CalcTextSize(Text, TextEnd);
        float  FrameSize = ImGui::GetStyle().FramePadding.y * 2.f;
        ImVec2 EndPos    = { Pos.x + Size.x + FrameSize, Pos.y + Size.y + FrameSize };

        DrawList->AddRectFilled(Pos, EndPos, ImGui::GetColorU32(ImGuiCol_FrameBg));
        DrawList->AddText({ Pos.x + FrameSize / 2.f, Pos.y + FrameSize / 2.f }, ImGui::GetColorU32(ImGuiCol_Text), Text, TextEnd);

        ImGui::Dummy(Size);
    }

    void IncrementCursorPosX(
        float Pos)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + Pos);
    }

    void IncrementCursorPosY(
        float Pos)
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + Pos);
    }

    void IncrementCursorPos(
        const ImVec2& Pos)
    {
        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + Pos.x, ImGui::GetCursorPosY() + Pos.y));
    }

    void TableAlignCenter(
        float Width)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() / 2.0f - Width / 2.0f);
    }

    void CenteredText(
        const char* Label)
    {
        float textW = ImGui::CalcTextSize(Label).x;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - textW / 2.0f);
        ImGui::Text(Label);
    }

    void CenterCursorX()
    {
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f);
    }

    void CenterCursorY()
    {
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f);
    }

    void CenterCursor()
    {
        auto Size = ImGui::GetWindowSize();
        ImGui::SetCursorPos(ImVec2(Size.x / 2.0f, Size.y / 2.0f));
    }

    //

    void DrawComponentLabel(
        const char* Label,
        bool        SameLine,
        float       PropertyNameWidth)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(Label);

        if (SameLine)
        {
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowSize().x * PropertyNameWidth);
        }
    }

    bool DrawColorPicker(
        const char*         Name,
        Color4&             Color,
        ImGuiColorEditFlags Flags)
    {
        imcxx::shared_item_id Id(Name);

        auto Col = std::bit_cast<ImVec4*>(glm::value_ptr(Color));
        if (imcxx::button{ imcxx::button::color{}, "", *Col, Flags, ImVec2{ ImGui::GetContentRegionAvail().x, 0.f } })
        {
            ImGui::OpenPopup("ColorPalette");
        }
        if (imcxx::popup ColorPopup{ "ColorPalette" })
        {
            return ImGui::ColorPicker4("", &Col->x, Flags);
        }
        return false;
    }

    bool DrawColorPicker(
        const char*         Name,
        Color3&             Color,
        ImGuiColorEditFlags Flags)
    {
        Color4 TmpCol(Color, 1.f);
        if (DrawColorPicker(Name, TmpCol, Flags | ImGuiColorEditFlags_NoAlpha))
        {
            Color = TmpCol;
            return true;
        }
        return false;
    }

    bool DrawColorPicker(
        const char*         Name,
        Color4U8&           Color,
        ImGuiColorEditFlags Flags)
    {
        Color4 TmpCol(Color.r / 255.f, Color.g / 255.f, Color.b / 255.f, Color.a / 255.f);
        if (DrawColorPicker(Name, TmpCol, Flags))
        {
            Color = TmpCol;
            return true;
        }
        return false;
    }

    bool DrawColorPicker(
        const char*         Name,
        Color3U8&           Color,
        ImGuiColorEditFlags Flags)
    {
        Color4 TmpCol(Color.r / 255.f, Color.g / 255.f, Color.b / 255.f, 1.f);
        if (DrawColorPicker(Name, TmpCol, Flags | ImGuiColorEditFlags_NoAlpha))
        {
            Color = TmpCol;
            return true;
        }
        return false;
    }
} // namespace Neon::UI::Utils