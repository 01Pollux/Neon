#pragma once

#include <UI/imcxx/all_in_one.hpp>
#include <UI/WindowUtils.hpp>
#include <UI/Fonts/FontAwesome5.hpp>

#include <Core/BitMask.hpp>
#include <Core/String.hpp>
#include <Math/Vector.hpp>
#include <span>

namespace Neon::UI::Utils
{
    /// <summary>
    /// Draw a text with a background color of ImGuiCol_FrameBg.
    /// </summary>
    template<StringType _StrTy>
    void DrawTextBG(
        const _StrTy& Text)
    {
        auto   DrawList  = ImGui::GetWindowDrawList();
        ImVec2 Pos       = ImGui::GetCursorScreenPos();
        ImVec2 Size      = ImGui::CalcTextSize(Text);
        float  FrameSize = ImGui::GetStyle().FramePadding.y * 2.f;
        ImVec2 EndPos    = { Pos.x + Size.x + FrameSize, Pos.y + Size.y + FrameSize };

        DrawList->AddRectFilled(Pos, EndPos, ImGui::GetColorU32(ImGuiCol_FrameBg));
        DrawList->AddText({ Pos.x + FrameSize / 2.f, Pos.y + FrameSize / 2.f }, ImGui::GetColorU32(ImGuiCol_Text), Text);

        ImGui::Dummy(Size);
    }

    //

    /// <summary>
    /// Increment the cursor position on the X axis.
    /// </summary>
    void IncrementCursorPosX(
        float Pos);

    /// <summary>
    /// Increment the cursor position on the Y axis.
    /// </summary>
    void IncrementCursorPosY(
        float Pos);

    /// <summary>
    /// Increment the cursor position.
    /// </summary>
    void IncrementCursorPos(
        const ImVec2& Pos);

    /// <summary>
    /// Align the cursor position on the X axis.
    /// </summary>
    void TableAlignCenter(
        float Width);

    /// <summary>
    /// Write a text at the center of the window.
    /// </summary>
    void CenteredText(
        const char* Label);

    /// <summary>
    /// Center the cursor position on the X axis.
    /// </summary>
    void CenterCursorX();

    /// <summary>
    /// Center the cursor position on the Y axis.
    /// </summary>
    void CenterCursorY();

    /// <summary>
    /// Center the cursor position.
    /// </summary>
    void CenterCursor();

    //

    // Properties
    enum class ComponentActionType : uint8_t
    {
        Remove,
        Copy,
        Paste,
        Reset,

        _Last_Enum
    };

    struct ComponentHeaderInfo
    {
        Bitmask<ComponentActionType> AllowedActions;
        ComponentActionType          FinalAction : 2 {};
        bool                         IsOpen      : 1 = false;

        operator bool() const noexcept
        {
            return IsOpen;
        }
    };

    /// <summary>
    /// Begins a component header with the given label and togglable state.
    /// </summary>
    [[nodiscard]] static ComponentHeaderInfo BeginComponentHeader(
        const char* Label,
        bool*       Togglable = nullptr)
    {
        ComponentHeaderInfo Ret;
        const bool          IsActive = Togglable ? *Togglable : true;

        // Check if the component is disabled
        if (!IsActive)
        {
            ImGui::BeginDisabled();
        }

        // Draw the header
        Ret.IsOpen = ImGui::TreeNodeEx(Label, ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_Framed);

        if (!IsActive)
        {
            ImGui::EndDisabled();
        }

        // Draw the settings button
        float FrameHeight = ImGui::GetFrameHeight();
        ImGui::SameLine(ImGui::GetWindowWidth() - FrameHeight);

        // Draw the settings button
        if (ImGui::Button(ICON_FA_COG, { FrameHeight, FrameHeight }))
        {
            ImGui::OpenPopup("##ComponentSettings");
        }

        // Draw the settings popup
        if (ImGui::BeginPopup("##ComponentSettings"))
        {
            for (auto& [ButtonName, Action] : {
                     std::pair{ "Copy", ComponentActionType::Copy },
                     std::pair{ "Paste", ComponentActionType::Paste },
                     std::pair{ "Reset", ComponentActionType::Reset },
                     std::pair{ "Remove", ComponentActionType::Remove } })
            {
                bool ActionAllowed = Ret.AllowedActions.Test(Action);

                imcxx::disabled Disabled(ActionAllowed);
                if (ImGui::MenuItem(ButtonName))
                {
                    if (ActionAllowed)
                    {
                        Ret.FinalAction = Action;
                    }
                }
            }
            ImGui::EndPopup();
        }

        return Ret;
    }

    /// <summary>
    /// Ends a component header.
    /// </summary>
    static void EndComponentHeader()
    {
        ImGui::TreePop();
    }

    /// <summary>
    /// Draws a component label with the given tooltip.
    /// </summary>
    void DrawComponentLabel(
        const char* Label,
        bool        SameLine          = true,
        float       PropertyNameWidth = 0.25f);

    enum class DrawVectorType : uint8_t
    {
        Drag,
        Input,
        Slider
    };

    static constexpr const char* DrawVectorPositionNames[] = {
        "X",
        "Y",
        "Z",
        "W"
    };

    static constexpr const char* DrawVectorColorNames[] = {
        "R",
        "G",
        "B",
        "A"
    };

    static constexpr const char* DrawVectorRotationNames[] = {
        "Pitch",
        "Yaw",
        "Roll"
    };

    template<Vec::VectorType _Ty>
    struct DrawVectorData
    {
        _Ty& Value = nullptr;

        typename _Ty::value_type Min = _Ty::value_type(0);
        typename _Ty::value_type Max = _Ty::value_type(0);

        /// <summary>
        /// Flags are either ImGuiSliderFlags for slider+drag or ImGuiInputTextFlags for input.
        /// </summary>
        uint32_t Flags = 0;

        /// <summary>
        /// Names must be an array of _Ty::length strings.
        /// </summary>
        std::span<const char* const> Names         = DrawVectorPositionNames;
        std::optional<ImVec4>        PropertyColor = ImVec4{ 0.47f, 0.19f, 0.19f, 1.f };
    };

    /// <summary>
    /// Draw a vector component.
    ///
    /// If Format is nullptr, the default format is used.
    /// If Flags is 0, the default flags are used.
    /// Flags are either ImGuiSliderFlags for slider+drag or ImGuiInputTextFlags for input.
    /// </summary>
    template<DrawVectorType DrawType, Vec::VectorType _Ty>
    static bool DrawVectorComponent(
        DrawVectorData<_Ty> DrawData)
    {
        ImGui::PushID(std::addressof(DrawData.Value));
        bool Changed = false;

        float TextSize = 0.f;
        for (typename _Ty::length_type i = 0; i < _Ty::length(); i++)
        {
            TextSize += ImGui::CalcTextSize(DrawData.Names[i]).x;
        }

        float RegionWidth = (ImGui::GetContentRegionAvail().x - TextSize) / _Ty::length() - ImGui::GetStyle().ItemSpacing.x * 2.f;

        for (typename _Ty::length_type i = 0; i < _Ty::length(); i++)
        {
            if (DrawData.PropertyColor) [[likely]]
            {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, *DrawData.PropertyColor);
            }
            DrawTextBG(DrawData.Names[i]);
            if (DrawData.PropertyColor) [[likely]]
            {
                ImGui::PopStyleColor();
            }

            ImGui::SetNextItemWidth(RegionWidth);
            ImGui::SameLine();
            ImGui::PushID(i);

            if constexpr (DrawType == DrawVectorType::Drag)
            {
                if constexpr (std::is_same_v<typename _Ty::value_type, float>)
                {
                    Changed |= ImGui::DragFloat("", &DrawData.Value[i], 0.1f, DrawData.Min, DrawData.Max);
                }
                else if constexpr (std::is_same_v<typename _Ty::value_type, int32_t> ||
                                   std::is_same_v<typename _Ty::value_type, uint32_t>)
                {
                    Changed |= ImGui::DragInt("", std::bit_cast<int*>(&DrawData.Value[i]), 1, DrawData.Min, DrawData.Max);
                }
            }
            else if constexpr (DrawType == DrawVectorType::Input)
            {
                if constexpr (std::is_same_v<typename _Ty::value_type, float>)
                {
                    Changed |= ImGui::InputFloat("", &DrawData.Value[i]);
                    if (DrawData.Min < DrawData.Max)
                    {
                        DrawData.Value[i] = std::clamp(DrawData.Value[i], DrawData.Min, DrawData.Max);
                    }
                }
                else if constexpr (std::is_same_v<typename _Ty::value_type, int32_t> ||
                                   std::is_same_v<typename _Ty::value_type, uint32_t>)
                {
                    Changed |= ImGui::InputInt("", std::bit_cast<int*>(&DrawData.Value[i]));
                    if (DrawData.Min < DrawData.Max)
                    {
                        DrawData.Value[i] = std::clamp(DrawData.Value[i], DrawData.Min, DrawData.Max);
                    }
                }
            }
            else if constexpr (DrawType == DrawVectorType::Slider)
            {
                if constexpr (std::is_same_v<typename _Ty::value_type, float>)
                {
                    Changed |= ImGui::SliderFloat("", &DrawData.Value[i], DrawData.Min, DrawData.Max);
                }
                else if constexpr (std::is_same_v<typename _Ty::value_type, int32_t> ||
                                   std::is_same_v<typename _Ty::value_type, uint32_t>)
                {
                    Changed |= ImGui::SliderInt("", std::bit_cast<int*>(&DrawData.Value[i]), DrawData.Min, DrawData.Max);
                }
            }

            ImGui::PopID();
            if (i != (_Ty::length() - 1))
            {
                ImGui::SameLine();
            }
        }
        ImGui::PopID();
        return Changed;
    }

    /// <summary>
    /// Draw a vector component.
    ///
    /// If Format is nullptr, the default format is used.
    /// If Flags is 0, the default flags are used.
    /// Flags are either ImGuiSliderFlags for slider+drag or ImGuiInputTextFlags for input.
    /// </summary>
    template<Vec::VectorType _Ty>
    static bool DragVectorComponent(
        DrawVectorData<_Ty> DrawData)
    {
        return DrawVectorComponent<DrawVectorType::Drag>(std::move(DrawData));
    }

    /// <summary>
    /// Draw a vector component.
    ///
    /// If Format is nullptr, the default format is used.
    /// If Flags is 0, the default flags are used.
    /// Flags are either ImGuiSliderFlags for slider+drag or ImGuiInputTextFlags for input.
    /// </summary>
    template<Vec::VectorType _Ty>
    static bool DragVectorComponent(
        _Ty& Value)
    {
        return DrawVectorComponent<DrawVectorType::Drag>(DrawVectorData(Value));
    }

    /// <summary>
    /// Draw a vector component.
    ///
    /// If Format is nullptr, the default format is used.
    /// If Flags is 0, the default flags are used.
    /// Flags are either ImGuiSliderFlags for slider+drag or ImGuiInputTextFlags for input.
    /// </summary>
    template<Vec::VectorType _Ty>
    static bool InputVectorComponent(
        DrawVectorData<_Ty> DrawData)
    {
        return DrawVectorComponent<DrawVectorType::Input>(std::move(DrawData));
    }

    /// <summary>
    /// Draw a vector component.
    ///
    /// If Format is nullptr, the default format is used.
    /// If Flags is 0, the default flags are used.
    /// Flags are either ImGuiSliderFlags for slider+drag or ImGuiInputTextFlags for input.
    /// </summary>
    template<Vec::VectorType _Ty>
    static bool InputVectorComponent(
        _Ty& Value)
    {
        return DrawVectorComponent<DrawVectorType::Input>(DrawVectorData(Value));
    }

    /// <summary>
    /// Draw a vector component.
    ///
    /// If Format is nullptr, the default format is used.
    /// If Flags is 0, the default flags are used.
    /// Flags are either ImGuiSliderFlags for slider+drag or ImGuiInputTextFlags for input.
    /// </summary>
    template<Vec::VectorType _Ty>
    static bool SliderVectorComponent(
        DrawVectorData<_Ty> DrawData)
    {
        return DrawVectorComponent<DrawVectorType::Slider>(std::move(DrawData));
    }

    /// <summary>
    /// Draw a vector component.
    ///
    /// If Format is nullptr, the default format is used.
    /// If Flags is 0, the default flags are used.
    /// Flags are either ImGuiSliderFlags for slider+drag or ImGuiInputTextFlags for input.
    /// </summary>
    template<Vec::VectorType _Ty>
    static bool SliderVectorComponent(
        _Ty& Value)
    {
        return DrawVectorComponent<DrawVectorType::Slider>(DrawVectorData(Value));
    }
} // namespace Neon::UI::Utils