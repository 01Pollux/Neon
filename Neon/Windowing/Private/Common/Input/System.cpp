#include <WindowPCH.hpp>
#include <Input/System.hpp>

#include <ImGui/imgui.h>

namespace Neon::Input
{
    /// <summary>
    /// Convert a Neon key to an ImGui key.
    /// </summary>
    [[nodiscard]] ImGuiKey GetImGuiKey(
        EKeyboardInput Type)
    {
        switch (Type)
        {
        case EKeyboardInput::LeftArrow:
            return ImGuiKey_LeftArrow;
        case EKeyboardInput::RightArrow:
            return ImGuiKey_RightArrow;
        case EKeyboardInput::UpArrow:
            return ImGuiKey_UpArrow;
        case EKeyboardInput::DownArrow:
            return ImGuiKey_DownArrow;
        case EKeyboardInput::PageUp:
            return ImGuiKey_PageUp;
        case EKeyboardInput::PageDown:
            return ImGuiKey_PageDown;
        case EKeyboardInput::Home:
            return ImGuiKey_Home;
        case EKeyboardInput::End:
            return ImGuiKey_End;
        case EKeyboardInput::Insert:
            return ImGuiKey_Insert;
        case EKeyboardInput::Delete:
            return ImGuiKey_Delete;
        case EKeyboardInput::Backspace:
            return ImGuiKey_Backspace;
        case EKeyboardInput::Space:
            return ImGuiKey_Space;
        case EKeyboardInput::Enter:
            return ImGuiKey_Enter;
        case EKeyboardInput::Tab:
            return ImGuiKey_Tab;
        case EKeyboardInput::Escape:
            return ImGuiKey_Escape;
        case EKeyboardInput::LCtrl:
            return ImGuiKey_LeftCtrl;
        case EKeyboardInput::LShift:
            return ImGuiKey_LeftShift;
        case EKeyboardInput::LAlt:
            return ImGuiKey_LeftAlt;
        case EKeyboardInput::RCtrl:
            return ImGuiKey_RightCtrl;
        case EKeyboardInput::RShift:
            return ImGuiKey_RightShift;
        case EKeyboardInput::RAlt:
            return ImGuiKey_RightAlt;
        case EKeyboardInput::LSuper:
            return ImGuiKey_LeftSuper;
        case EKeyboardInput::RSuper:
            return ImGuiKey_RightSuper;
        case EKeyboardInput::Menu:
            return ImGuiKey_Menu;
        case EKeyboardInput::Keypad0:
            return ImGuiKey_Keypad0;
        case EKeyboardInput::Keypad1:
            return ImGuiKey_Keypad1;
        case EKeyboardInput::Keypad2:
            return ImGuiKey_Keypad2;
        case EKeyboardInput::Keypad3:
            return ImGuiKey_Keypad3;
        case EKeyboardInput::Keypad4:
            return ImGuiKey_Keypad4;
        case EKeyboardInput::Keypad5:
            return ImGuiKey_Keypad5;
        case EKeyboardInput::Keypad6:
            return ImGuiKey_Keypad6;
        case EKeyboardInput::Keypad7:
            return ImGuiKey_Keypad7;
        case EKeyboardInput::Keypad8:
            return ImGuiKey_Keypad8;
        case EKeyboardInput::Keypad9:
            return ImGuiKey_Keypad9;
        case EKeyboardInput::_0:
            return ImGuiKey_0;
        case EKeyboardInput::_1:
            return ImGuiKey_1;
        case EKeyboardInput::_2:
            return ImGuiKey_2;
        case EKeyboardInput::_3:
            return ImGuiKey_3;
        case EKeyboardInput::_4:
            return ImGuiKey_4;
        case EKeyboardInput::_5:
            return ImGuiKey_5;
        case EKeyboardInput::_6:
            return ImGuiKey_6;
        case EKeyboardInput::_7:
            return ImGuiKey_7;
        case EKeyboardInput::_8:
            return ImGuiKey_8;
        case EKeyboardInput::_9:
            return ImGuiKey_9;
        case EKeyboardInput::A:
            return ImGuiKey_A;
        case EKeyboardInput::B:
            return ImGuiKey_B;
        case EKeyboardInput::C:
            return ImGuiKey_C;
        case EKeyboardInput::D:
            return ImGuiKey_D;
        case EKeyboardInput::E:
            return ImGuiKey_E;
        case EKeyboardInput::F:
            return ImGuiKey_F;
        case EKeyboardInput::G:
            return ImGuiKey_G;
        case EKeyboardInput::H:
            return ImGuiKey_H;
        case EKeyboardInput::I:
            return ImGuiKey_I;
        case EKeyboardInput::J:
            return ImGuiKey_J;
        case EKeyboardInput::K:
            return ImGuiKey_K;
        case EKeyboardInput::L:
            return ImGuiKey_L;
        case EKeyboardInput::M:
            return ImGuiKey_M;
        case EKeyboardInput::N:
            return ImGuiKey_N;
        case EKeyboardInput::O:
            return ImGuiKey_O;
        case EKeyboardInput::P:
            return ImGuiKey_P;
        case EKeyboardInput::Q:
            return ImGuiKey_Q;
        case EKeyboardInput::R:
            return ImGuiKey_R;
        case EKeyboardInput::S:
            return ImGuiKey_S;
        case EKeyboardInput::T:
            return ImGuiKey_T;
        case EKeyboardInput::U:
            return ImGuiKey_U;
        case EKeyboardInput::V:
            return ImGuiKey_V;
        case EKeyboardInput::W:
            return ImGuiKey_W;
        case EKeyboardInput::X:
            return ImGuiKey_X;
        case EKeyboardInput::Y:
            return ImGuiKey_Y;
        case EKeyboardInput::Z:
            return ImGuiKey_Z;
        case EKeyboardInput::F1:
            return ImGuiKey_F1;
        case EKeyboardInput::F2:
            return ImGuiKey_F2;
        case EKeyboardInput::F3:
            return ImGuiKey_F3;
        case EKeyboardInput::F4:
            return ImGuiKey_F4;
        case EKeyboardInput::F5:
            return ImGuiKey_F5;
        case EKeyboardInput::F6:
            return ImGuiKey_F6;
        case EKeyboardInput::F7:
            return ImGuiKey_F7;
        case EKeyboardInput::F8:
            return ImGuiKey_F8;
        case EKeyboardInput::F9:
            return ImGuiKey_F9;
        case EKeyboardInput::F10:
            return ImGuiKey_F10;
        case EKeyboardInput::F11:
            return ImGuiKey_F11;
        case EKeyboardInput::F12:
            return ImGuiKey_F12;
        case EKeyboardInput::Apostrophe:
            return ImGuiKey_Apostrophe;
        case EKeyboardInput::Comma:
            return ImGuiKey_Comma;
        case EKeyboardInput::Minus:
            return ImGuiKey_Minus;
        case EKeyboardInput::Period:
            return ImGuiKey_Period;
        case EKeyboardInput::Slash:
            return ImGuiKey_Slash;
        case EKeyboardInput::Semicolon:
            return ImGuiKey_Semicolon;
        case EKeyboardInput::Equal:
            return ImGuiKey_Equal;
        case EKeyboardInput::Backslash:
            return ImGuiKey_Backslash;
        case EKeyboardInput::LeftBracket:
            return ImGuiKey_LeftBracket;
        case EKeyboardInput::RightBracket:
            return ImGuiKey_RightBracket;
        case EKeyboardInput::GraveAccent:
            return ImGuiKey_GraveAccent;
        case EKeyboardInput::CapsLock:
            return ImGuiKey_CapsLock;
        case EKeyboardInput::ScrollLock:
            return ImGuiKey_ScrollLock;
        case EKeyboardInput::NumLock:
            return ImGuiKey_NumLock;
        case EKeyboardInput::PrintScreen:
            return ImGuiKey_PrintScreen;
        case EKeyboardInput::Pause:
            return ImGuiKey_Pause;
        case EKeyboardInput::KeypadDecimal:
            return ImGuiKey_KeypadDecimal;
        case EKeyboardInput::KeypadDivide:
            return ImGuiKey_KeypadDivide;
        case EKeyboardInput::KeypadMultiply:
            return ImGuiKey_KeypadMultiply;
        case EKeyboardInput::KeypadSubtract:
            return ImGuiKey_KeypadSubtract;
        case EKeyboardInput::KeypadAdd:
            return ImGuiKey_KeypadAdd;
        case EKeyboardInput::KeypadEnter:
            return ImGuiKey_KeypadEnter;
        default:
            std::unreachable();
        }
    }

    [[nodiscard]] ImGuiMouseButton GetImGuiMouseButton(
        EMouseInput Type)
    {
        switch (Type)
        {
        case EMouseInput::Left:
            return ImGuiMouseButton_Left;
        case EMouseInput::Right:
            return ImGuiMouseButton_Right;
        case EMouseInput::Middle:
            return ImGuiMouseButton_Middle;
        case EMouseInput::Button4:
            return ImGuiMouseButton_Middle + 1;
        case EMouseInput::Button5:
            return ImGuiMouseButton_Middle + 2;
        default:
            std::unreachable();
        }
    }

    //

    bool IsKeyPressed(
        EKeyboardInput Type)
    {
        return ImGui::IsKeyPressed(GetImGuiKey(Type));
    }

    bool IsKeyDown(
        EKeyboardInput Type)
    {
        return ImGui::IsKeyDown(GetImGuiKey(Type));
    }

    bool IsKeyUp(
        EKeyboardInput Type)
    {
        return ImGui::IsKeyReleased(GetImGuiKey(Type));
    }

    //

    bool IsMouseDown(
        EMouseInput Type)
    {
        return ImGui::IsMouseDown(GetImGuiMouseButton(Type));
    }

    bool IsAnyMouseDown()
    {
        return ImGui::IsAnyMouseDown();
    }

    bool IsMouseUp(
        EMouseInput Type)
    {
        return ImGui::IsMouseReleased(GetImGuiMouseButton(Type));
    }

    uint32_t GetMouseClickedCount(
        EMouseInput Type)
    {
        return ImGui::GetMouseClickedCount(GetImGuiMouseButton(Type));
    }

    bool IsMouseClicked(
        EMouseInput Type,
        bool        Repeat)
    {
        return ImGui::IsMouseClicked(GetImGuiMouseButton(Type), Repeat);
    }

    bool IsMouseDoubleClicked(
        EMouseInput Type)
    {
        return ImGui::IsMouseDoubleClicked(GetImGuiMouseButton(Type));
    }

    bool IsMouseHoveringRect(
        const RectF& Rect)
    {
        return ImGui::IsMouseHoveringRect(
            ImVec2(Rect.Left(), Rect.Top()),
            ImVec2(Rect.Right(), Rect.Bottom()));
    }

    Vector2 GetMousePos()
    {
        auto Pos = ImGui::GetMousePos();
        return Vector2(Pos.x, Pos.y);
    }
} // namespace Neon::Input