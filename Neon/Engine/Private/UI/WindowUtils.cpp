#include <EnginePCH.hpp>
#include <UI/Utils.hpp>

namespace Neon::UI::WindowUtils
{
    // From imgui.cpp
    void RenderWindowOuterBorders(
        ImGuiWindow* Window)
    {
        // Data for resizing from borders
        struct ImGuiResizeBorderDef
        {
            ImVec2 InnerDir;
            ImVec2 SegmentN1, SegmentN2;
            float  OuterAngle;
        };

        constexpr ImGuiResizeBorderDef ResizeBorderDef[4] = {
            { ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f }, // Left
            { ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f }, // Right
            { ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f }, // Up
            { ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }  // Down
        };

        auto GetResizeBorderRect = [](ImGuiWindow* Window, ImGuiDir BorderN, float perp_padding, float Thickness)
        {
            ImRect Rect = Window->Rect();
            if (Thickness == 0.0f)
                Rect.Max -= ImVec2(1, 1);
            switch (BorderN)
            {
            case ImGuiDir_Left:
                return ImRect(Rect.Min.x - Thickness, Rect.Min.y + perp_padding, Rect.Min.x + Thickness, Rect.Max.y - perp_padding);
            case ImGuiDir_Right:
                return ImRect(Rect.Max.x - Thickness, Rect.Min.y + perp_padding, Rect.Max.x + Thickness, Rect.Max.y - perp_padding);
            case ImGuiDir_Up:
                return ImRect(Rect.Min.x + perp_padding, Rect.Min.y - Thickness, Rect.Max.x - perp_padding, Rect.Min.y + Thickness);
            case ImGuiDir_Down:
                return ImRect(Rect.Min.x + perp_padding, Rect.Max.y - Thickness, Rect.Max.x - perp_padding, Rect.Max.y + Thickness);
            default:
                std::unreachable();
            }
        };

        //

        float Rounding   = Window->WindowRounding;
        float BorderSize = Window->WindowBorderSize;
        if (BorderSize > 0.0f && !(Window->Flags & ImGuiWindowFlags_NoBackground))
            Window->DrawList->AddRect(Window->Pos, Window->Pos + Window->Size, ImGui::GetColorU32(ImGuiCol_Border), Rounding, 0, BorderSize);

        int BorderHeld = Window->ResizeBorderHeld;
        if (BorderHeld != -1)
        {
            const auto& Def     = ResizeBorderDef[BorderHeld];
            ImRect      BorderR = GetResizeBorderRect(Window, BorderHeld, Rounding, 0.0f);
            Window->DrawList->PathArcTo(ImLerp(BorderR.Min, BorderR.Max, Def.SegmentN1) + ImVec2(0.5f, 0.5f) + Def.InnerDir * Rounding, Rounding, Def.OuterAngle - IM_PI * 0.25f, Def.OuterAngle);
            Window->DrawList->PathArcTo(ImLerp(BorderR.Min, BorderR.Max, Def.SegmentN2) + ImVec2(0.5f, 0.5f) + Def.InnerDir * Rounding, Rounding, Def.OuterAngle, Def.OuterAngle + IM_PI * 0.25f);
            Window->DrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_SeparatorActive), 0, ImMax(2.0f, BorderSize)); // Thicker than usual
        }

        float FrameBorderSize = ImGui::GetStyle().FrameBorderSize;
        if (FrameBorderSize > 0 && !(Window->Flags & ImGuiWindowFlags_NoTitleBar) && !Window->DockIsActive)
        {
            float Y = Window->Pos.y + Window->TitleBarHeight() - 1;
            Window->DrawList->AddLine(ImVec2(Window->Pos.x + BorderSize, Y), ImVec2(Window->Pos.x + Window->Size.x - BorderSize, Y), ImGui::GetColorU32(ImGuiCol_Border), FrameBorderSize);
        }
    }
} // namespace Neon::UI::WindowUtils