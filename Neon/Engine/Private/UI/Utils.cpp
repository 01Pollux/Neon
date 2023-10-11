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
        ImVec2 RectSize  = ImGui::CalcTextSize(Text, TextEnd);
        float  FrameSize = ImGui::GetStyle().FramePadding.y * 2.f;
        ImVec2 EndPos    = { Pos.x + RectSize.x + FrameSize, Pos.y + RectSize.y + FrameSize };

        DrawList->AddRectFilled(Pos, EndPos, ImGui::GetColorU32(ImGuiCol_FrameBg));
        DrawList->AddText({ Pos.x + FrameSize / 2.f, Pos.y + FrameSize / 2.f }, ImGui::GetColorU32(ImGuiCol_Text), Text, TextEnd);

        ImGui::Dummy(RectSize);
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
        auto RectSize = ImGui::GetWindowSize();
        ImGui::SetCursorPos(ImVec2(RectSize.x / 2.0f, RectSize.y / 2.0f));
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

    //

    void DrawIcon(
        ImDrawList*   DrawList,
        const ImVec2& Size,
        BasicIconType Type,
        bool          Filled,
        ImU32         Color,
        ImU32         FillColor)
    {
        if (!ImGui::IsRectVisible(Size))
        {
            ImGui::Dummy(Size);
            return;
        }

        ImRect Rect(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + Size);
        ImVec2 RectSize      = Rect.Max - Rect.Min;
        ImVec2 RectCenter    = (Rect.Max + Rect.Min) * 0.5f;
        float  OutlineScale  = RectSize.x / 24.0f;
        int    ExtraSegments = int(2 * OutlineScale); // for full circle

        if (Type == BasicIconType::Flow)
        {
            auto Offset = -static_cast<int>(RectSize.x * 0.25f * 0.25f);

            Rect.Min.x += Offset;
            Rect.Max.x += Offset;
            RectCenter.x += Offset * 0.5f;
        }

        switch (Type)
        {
        case BasicIconType::Flow:
        {
            float Margin   = (Filled ? 2.0f : 2.0f) * OutlineScale;
            float Rounding = 0.1f * OutlineScale;
            float TipRound = 0.7f; // percentage of triangle edge (for tip)

            ImRect Canvas(
                Rect.Min.x + Margin + OutlineScale,
                Rect.Min.y + Margin,
                Rect.Max.x - Margin + OutlineScale,
                Rect.Max.y - Margin);
            float  CanvasX    = Canvas.Min.x;
            float  CanvasY    = Canvas.Min.y;
            ImVec2 CanvasSize = Canvas.Max - Canvas.Min;

            float left     = CanvasX + CanvasSize.x * 0.5f * 0.3f;
            float right    = CanvasX + CanvasSize.x - CanvasSize.x * 0.5f * 0.3f;
            float top      = CanvasY + CanvasSize.y * 0.5f * 0.2f;
            float bottom   = CanvasY + CanvasSize.y - CanvasSize.y * 0.5f * 0.2f;
            float center_y = (top + bottom) * 0.5f;

            ImVec2 tip_top(CanvasX + CanvasSize.x * 0.5f, top);
            ImVec2 tip_right(right, center_y);
            ImVec2 tip_bottom(CanvasX + CanvasSize.x * 0.5f, bottom);

            DrawList->PathLineTo(ImVec2(left, top) + ImVec2(0, Rounding));
            DrawList->PathBezierCubicCurveTo(
                ImVec2(left, top),
                ImVec2(left, top),
                ImVec2(left, top) + ImVec2(Rounding, 0));

            DrawList->PathLineTo(tip_top);
            DrawList->PathLineTo(tip_top + (tip_right - tip_top) * TipRound);
            DrawList->PathBezierCubicCurveTo(
                tip_right,
                tip_right,
                tip_bottom + (tip_right - tip_bottom) * TipRound);

            DrawList->PathLineTo(tip_bottom);
            DrawList->PathLineTo(ImVec2(left, bottom) + ImVec2(Rounding, 0));
            DrawList->PathBezierCubicCurveTo(
                ImVec2(left, bottom),
                ImVec2(left, bottom),
                ImVec2(left, bottom) - ImVec2(0, Rounding));

            if (!Filled)
            {
                if (FillColor & 0xFF000000)
                {
                    DrawList->AddConvexPolyFilled(DrawList->_Path.Data, DrawList->_Path.Size, FillColor);
                }
                DrawList->PathStroke(Color, true, 2.0f * OutlineScale);
            }
            else
            {
                DrawList->PathFillConvex(Color);
            }
            break;
        }

        case BasicIconType::Circle:
        {
            if (!Filled)
            {
                const auto Radius = 0.5f * RectSize.x / 2.0f - 0.5f;

                if (FillColor & 0xFF000000)
                {
                    DrawList->AddCircleFilled(RectCenter, Radius, FillColor, 12 + ExtraSegments);
                }
                DrawList->AddCircle(RectCenter, Radius, Color, 12 + ExtraSegments, 2.0f * OutlineScale);
            }
            else
            {
                DrawList->AddCircleFilled(RectCenter, 0.5f * RectSize.x / 2.0f, Color, 12 + ExtraSegments);
            }
            break;
        }

        case BasicIconType::Square:
        {
            if (Filled)
            {
                const auto Radius = 0.5f * RectSize.x / 2.0f;
                const auto p0     = RectCenter - ImVec2(Radius, Radius);
                const auto p1     = RectCenter + ImVec2(Radius, Radius);

                DrawList->AddRectFilled(p0, p1, Color, 0, ImDrawFlags_RoundCornersAll);
            }
            else
            {
                const auto Radius = 0.5f * RectSize.x / 2.0f - 0.5f;
                const auto p0     = RectCenter - ImVec2(Radius, Radius);
                const auto p1     = RectCenter + ImVec2(Radius, Radius);

                if (FillColor & 0xFF000000)
                {
                    DrawList->AddRectFilled(p0, p1, FillColor, 0, ImDrawFlags_RoundCornersAll);
                }
                DrawList->AddRect(p0, p1, Color, 0, ImDrawFlags_RoundCornersAll, 2.0f * OutlineScale);
            }
            break;
        }

        case BasicIconType::Grid:
        {
            const auto Radius = 0.5f * RectSize.x / 2.0f;
            const auto w      = ceilf(Radius / 3.0f);

            const auto BaseTl = ImVec2(floorf(RectCenter.x - w * 2.5f), floorf(RectCenter.y - w * 2.5f));
            const auto BaseBr = ImVec2(floorf(BaseTl.x + w), floorf(BaseTl.y + w));

            auto tl = BaseTl;
            auto br = BaseBr;
            for (int i = 0; i < 3; ++i)
            {
                tl.x = BaseTl.x;
                br.x = BaseBr.x;
                DrawList->AddRectFilled(tl, br, Color);
                tl.x += w * 2;
                br.x += w * 2;
                if (i != 1 || Filled)
                    DrawList->AddRectFilled(tl, br, Color);
                tl.x += w * 2;
                br.x += w * 2;
                DrawList->AddRectFilled(tl, br, Color);

                tl.y += w * 2;
                br.y += w * 2;
            }
            break;
        }

        case BasicIconType::RoundSquare:
        {
            if (Filled)
            {
                const auto Radius = 0.5f * RectSize.x / 2.0f;
                const auto p0     = RectCenter - ImVec2(Radius, Radius);
                const auto p1     = RectCenter + ImVec2(Radius, Radius);

                DrawList->AddRectFilled(p0, p1, Color, Radius * 0.5f, ImDrawFlags_RoundCornersAll);
            }
            else
            {
                const auto Radius = 0.5f * RectSize.x / 2.0f - 0.5f;
                const auto p0     = RectCenter - ImVec2(Radius, Radius);
                const auto p1     = RectCenter + ImVec2(Radius, Radius);

                if (FillColor & 0xFF000000)
                {
                    DrawList->AddRectFilled(p0, p1, FillColor, Radius * 0.5f, ImDrawFlags_RoundCornersAll);
                }
                DrawList->AddRect(p0, p1, Color, Radius * 0.5f, ImDrawFlags_RoundCornersAll, 2.0f * OutlineScale);
            }
            break;
        }

        case BasicIconType::Diamond:
        {
            if (Filled)
            {
                const auto Radius = 0.607f * RectSize.x / 2.0f;
                DrawList->PathLineTo(RectCenter + ImVec2(0, -Radius));
                DrawList->PathLineTo(RectCenter + ImVec2(Radius, 0));
                DrawList->PathLineTo(RectCenter + ImVec2(0, Radius));
                DrawList->PathLineTo(RectCenter + ImVec2(-Radius, 0));
                DrawList->PathFillConvex(Color);
            }
            else
            {
                const auto Radius = 0.607f * RectSize.x / 2.0f - 0.5f;
                DrawList->PathLineTo(RectCenter + ImVec2(0, -Radius));
                DrawList->PathLineTo(RectCenter + ImVec2(Radius, 0));
                DrawList->PathLineTo(RectCenter + ImVec2(0, Radius));
                DrawList->PathLineTo(RectCenter + ImVec2(-Radius, 0));

                if (FillColor & 0xFF000000)
                {
                    DrawList->AddConvexPolyFilled(DrawList->_Path.Data, DrawList->_Path.Size, FillColor);
                }
                DrawList->PathStroke(Color, true, 2.0f * OutlineScale);
            }
            break;
        }
        default:
            std::unreachable();
        }

        ImGui::Dummy(Size);
    }
} // namespace Neon::UI::Utils
