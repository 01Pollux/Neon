#include <EnginePCH.hpp>
#include <UI/Utils.hpp>

namespace Neon::UI::Utils
{
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
} // namespace Neon::UI::Utils