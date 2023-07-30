#include <WindowPCH.hpp>
#include <Private/Windows/Input/Types.hpp>
#include <Private/Windows/API/WindowHeaders.hpp>

#define WM_KEYPAD_RETURN (WM_USER + VK_RETURN)

namespace Neon::Input
{
    unsigned int KeyboardInputToPlatform(
        EKeyboardInput KeyCode)
    {
        switch (KeyCode)
        {
        case EKeyboardInput::LeftArrow:
            return VK_LEFT;
        case EKeyboardInput::RightArrow:
            return VK_RIGHT;
        case EKeyboardInput::UpArrow:
            return VK_UP;
        case EKeyboardInput::DownArrow:
            return VK_DOWN;

        case EKeyboardInput::PageUp:
            return VK_PRIOR;
        case EKeyboardInput::PageDown:
            return VK_NEXT;
        case EKeyboardInput::Home:
            return VK_HOME;
        case EKeyboardInput::End:
            return VK_END;
        case EKeyboardInput::Insert:
            return VK_INSERT;
        case EKeyboardInput::Delete:
            return VK_DELETE;
        case EKeyboardInput::Backspace:
            return VK_BACK;
        case EKeyboardInput::Space:
            return VK_SPACE;
        case EKeyboardInput::Enter:
            return VK_RETURN;
        case EKeyboardInput::Tab:
            return VK_TAB;
        case EKeyboardInput::Escape:
            return VK_ESCAPE;
        case EKeyboardInput::LeftCtrl:
            return VK_LCONTROL;
        case EKeyboardInput::LeftShift:
            return VK_LSHIFT;
        case EKeyboardInput::LeftAlt:
            return VK_LMENU;
        case EKeyboardInput::LeftSuper:
            return VK_LWIN;
        case EKeyboardInput::RightCtrl:
            return VK_RCONTROL;
        case EKeyboardInput::RightShift:
            return VK_RSHIFT;
        case EKeyboardInput::RightAlt:
            return VK_RMENU;
        case EKeyboardInput::RightSuper:
            return VK_RWIN;
        case EKeyboardInput::Menu:
            return VK_APPS;

        case EKeyboardInput::Keypad0:
            return VK_NUMPAD0;
        case EKeyboardInput::Keypad1:
            return VK_NUMPAD1;
        case EKeyboardInput::Keypad2:
            return VK_NUMPAD2;
        case EKeyboardInput::Keypad3:
            return VK_NUMPAD3;
        case EKeyboardInput::Keypad4:
            return VK_NUMPAD4;
        case EKeyboardInput::Keypad5:
            return VK_NUMPAD5;
        case EKeyboardInput::Keypad6:
            return VK_NUMPAD6;
        case EKeyboardInput::Keypad7:
            return VK_NUMPAD7;
        case EKeyboardInput::Keypad8:
            return VK_NUMPAD8;
        case EKeyboardInput::Keypad9:
            return VK_NUMPAD9;

        case EKeyboardInput::_0:
            return '0';
        case EKeyboardInput::_1:
            return '1';
        case EKeyboardInput::_2:
            return '2';
        case EKeyboardInput::_3:
            return '3';
        case EKeyboardInput::_4:
            return '4';
        case EKeyboardInput::_5:
            return '5';
        case EKeyboardInput::_6:
            return '6';
        case EKeyboardInput::_7:
            return '7';
        case EKeyboardInput::_8:
            return '8';
        case EKeyboardInput::_9:
            return '9';

        case EKeyboardInput::A:
            return 'A';
        case EKeyboardInput::B:
            return 'B';
        case EKeyboardInput::C:
            return 'C';
        case EKeyboardInput::D:
            return 'D';
        case EKeyboardInput::E:
            return 'E';
        case EKeyboardInput::F:
            return 'F';
        case EKeyboardInput::G:
            return 'G';
        case EKeyboardInput::H:
            return 'H';
        case EKeyboardInput::I:
            return 'I';
        case EKeyboardInput::J:
            return 'J';
        case EKeyboardInput::K:
            return 'K';
        case EKeyboardInput::L:
            return 'L';
        case EKeyboardInput::M:
            return 'M';
        case EKeyboardInput::N:
            return 'N';
        case EKeyboardInput::O:
            return 'O';
        case EKeyboardInput::P:
            return 'P';
        case EKeyboardInput::Q:
            return 'Q';
        case EKeyboardInput::R:
            return 'R';
        case EKeyboardInput::S:
            return 'S';
        case EKeyboardInput::T:
            return 'T';
        case EKeyboardInput::U:
            return 'U';
        case EKeyboardInput::V:
            return 'V';
        case EKeyboardInput::W:
            return 'W';
        case EKeyboardInput::X:
            return 'X';
        case EKeyboardInput::Y:
            return 'Y';
        case EKeyboardInput::Z:
            return 'Z';

        case EKeyboardInput::F1:
            return VK_F1;
        case EKeyboardInput::F2:
            return VK_F2;
        case EKeyboardInput::F3:
            return VK_F3;
        case EKeyboardInput::F4:
            return VK_F4;
        case EKeyboardInput::F5:
            return VK_F5;
        case EKeyboardInput::F6:
            return VK_F6;
        case EKeyboardInput::F7:
            return VK_F7;
        case EKeyboardInput::F8:
            return VK_F8;
        case EKeyboardInput::F9:
            return VK_F9;
        case EKeyboardInput::F10:
            return VK_F10;
        case EKeyboardInput::F11:
            return VK_F11;
        case EKeyboardInput::F12:
            return VK_F12;

        case EKeyboardInput::Apostrophe:
            return VK_OEM_7;
        case EKeyboardInput::Comma:
            return VK_OEM_COMMA;
        case EKeyboardInput::Minus:
            return VK_OEM_MINUS;
        case EKeyboardInput::Period:
            return VK_OEM_PERIOD;
        case EKeyboardInput::Slash:
            return VK_OEM_2;
        case EKeyboardInput::Semicolon:
            return VK_OEM_1;
        case EKeyboardInput::Equal:
            return VK_OEM_PLUS;
        case EKeyboardInput::LeftBracket:
            return VK_OEM_4;
        case EKeyboardInput::Backslash:
            return VK_OEM_5;
        case EKeyboardInput::RightBracket:
            return VK_OEM_6;
        case EKeyboardInput::GraveAccent:
            return VK_OEM_3;
        case EKeyboardInput::CapsLock:
            return VK_CAPITAL;
        case EKeyboardInput::ScrollLock:
            return VK_SCROLL;
        case EKeyboardInput::NumLock:
            return VK_NUMLOCK;
        case EKeyboardInput::PrintScreen:
            return VK_SNAPSHOT;
        case EKeyboardInput::Pause:
            return VK_PAUSE;

        case EKeyboardInput::KeypadDecimal:
            return VK_DECIMAL;
        case EKeyboardInput::KeypadDivide:
            return VK_DIVIDE;
        case EKeyboardInput::KeypadMultiply:
            return VK_MULTIPLY;
        case EKeyboardInput::KeypadSubtract:
            return VK_SUBTRACT;
        case EKeyboardInput::KeypadAdd:
            return VK_ADD;
        case EKeyboardInput::KeypadEnter:
            return WM_KEYPAD_RETURN;
        }
        return 0;
    }

    EKeyboardInput KeyboardInputFromPlatform(
        unsigned long long KeyCode,
        unsigned long long lParam)
    {
        switch (KeyCode)
        {
        case VK_LEFT:
            return EKeyboardInput::LeftArrow;
        case VK_RIGHT:
            return EKeyboardInput::RightArrow;
        case VK_UP:
            return EKeyboardInput::UpArrow;
        case VK_DOWN:
            return EKeyboardInput::DownArrow;

        case VK_PRIOR:
            return EKeyboardInput::PageUp;
        case VK_NEXT:
            return EKeyboardInput::PageDown;
        case VK_HOME:
            return EKeyboardInput::Home;
        case VK_END:
            return EKeyboardInput::End;
        case VK_INSERT:
            return EKeyboardInput::Insert;
        case VK_DELETE:
            return EKeyboardInput::Delete;
        case VK_BACK:
            return EKeyboardInput::Backspace;
        case VK_SPACE:
            return EKeyboardInput::Space;
        case VK_RETURN:
            return (HIWORD(lParam) & KF_EXTENDED) ? EKeyboardInput::KeypadEnter : EKeyboardInput::Enter;
        case VK_TAB:
            return EKeyboardInput::Tab;
        case VK_ESCAPE:
            return EKeyboardInput::Escape;
        case VK_APPS:
            return EKeyboardInput::Menu;
        case VK_RWIN:
            return EKeyboardInput::RightSuper;
        case VK_LWIN:
            return EKeyboardInput::LeftSuper;
        case VK_MENU:
        {
            WORD ScanCode = (lParam >> 16) & 0xff;
            if (HIWORD(lParam) & KF_EXTENDED)
                ScanCode = MAKEWORD(ScanCode, 0xE0);

            return (LOWORD(MapVirtualKeyW(ScanCode, MAPVK_VSC_TO_VK_EX)) == VK_RMENU) ? EKeyboardInput::RightAlt : EKeyboardInput::LeftAlt;
        }
        case VK_CONTROL:
        {
            WORD ScanCode = (lParam >> 16) & 0xff;
            if (HIWORD(lParam) & KF_EXTENDED)
                ScanCode = MAKEWORD(ScanCode, 0xE0);

            return (LOWORD(MapVirtualKeyW(ScanCode, MAPVK_VSC_TO_VK_EX)) == VK_RCONTROL) ? EKeyboardInput::RightCtrl : EKeyboardInput::LeftCtrl;
        }
        case VK_SHIFT:
        {
            WORD ScanCode = (lParam >> 16) & 0xff;
            return (LOWORD(MapVirtualKeyW(ScanCode, MAPVK_VSC_TO_VK_EX)) == VK_RSHIFT) ? EKeyboardInput::RightShift : EKeyboardInput::LeftShift;
        }

        case VK_NUMPAD0:
            return EKeyboardInput::Keypad0;
        case VK_NUMPAD1:
            return EKeyboardInput::Keypad1;
        case VK_NUMPAD2:
            return EKeyboardInput::Keypad2;
        case VK_NUMPAD3:
            return EKeyboardInput::Keypad3;
        case VK_NUMPAD4:
            return EKeyboardInput::Keypad4;
        case VK_NUMPAD5:
            return EKeyboardInput::Keypad5;
        case VK_NUMPAD6:
            return EKeyboardInput::Keypad6;
        case VK_NUMPAD7:
            return EKeyboardInput::Keypad7;
        case VK_NUMPAD8:
            return EKeyboardInput::Keypad8;
        case VK_NUMPAD9:
            return EKeyboardInput::Keypad9;

        case '0':
            return EKeyboardInput::_0;
        case '1':
            return EKeyboardInput::_1;
        case '2':
            return EKeyboardInput::_2;
        case '3':
            return EKeyboardInput::_3;
        case '4':
            return EKeyboardInput::_4;
        case '5':
            return EKeyboardInput::_5;
        case '6':
            return EKeyboardInput::_6;
        case '7':
            return EKeyboardInput::_7;
        case '8':
            return EKeyboardInput::_8;
        case '9':
            return EKeyboardInput::_9;

        case 'A':
            return EKeyboardInput::A;
        case 'B':
            return EKeyboardInput::B;
        case 'C':
            return EKeyboardInput::C;
        case 'D':
            return EKeyboardInput::D;
        case 'E':
            return EKeyboardInput::E;
        case 'F':
            return EKeyboardInput::F;
        case 'G':
            return EKeyboardInput::G;
        case 'H':
            return EKeyboardInput::H;
        case 'I':
            return EKeyboardInput::I;
        case 'J':
            return EKeyboardInput::J;
        case 'K':
            return EKeyboardInput::K;
        case 'L':
            return EKeyboardInput::L;
        case 'M':
            return EKeyboardInput::M;
        case 'N':
            return EKeyboardInput::N;
        case 'O':
            return EKeyboardInput::O;
        case 'P':
            return EKeyboardInput::P;
        case 'Q':
            return EKeyboardInput::Q;
        case 'R':
            return EKeyboardInput::R;
        case 'S':
            return EKeyboardInput::S;
        case 'T':
            return EKeyboardInput::T;
        case 'U':
            return EKeyboardInput::U;
        case 'V':
            return EKeyboardInput::V;
        case 'W':
            return EKeyboardInput::W;
        case 'X':
            return EKeyboardInput::X;
        case 'Y':
            return EKeyboardInput::Y;
        case 'Z':
            return EKeyboardInput::Z;

        case VK_F1:
            return EKeyboardInput::F1;
        case VK_F2:
            return EKeyboardInput::F2;
        case VK_F3:
            return EKeyboardInput::F3;
        case VK_F4:
            return EKeyboardInput::F4;
        case VK_F5:
            return EKeyboardInput::F5;
        case VK_F6:
            return EKeyboardInput::F6;
        case VK_F7:
            return EKeyboardInput::F7;
        case VK_F8:
            return EKeyboardInput::F8;
        case VK_F9:
            return EKeyboardInput::F9;
        case VK_F10:
            return EKeyboardInput::F10;
        case VK_F11:
            return EKeyboardInput::F11;
        case VK_F12:
            return EKeyboardInput::F12;

        case VK_OEM_7:
            return EKeyboardInput::Apostrophe;
        case VK_OEM_COMMA:
            return EKeyboardInput::Comma;
        case VK_OEM_MINUS:
            return EKeyboardInput::Minus;
        case VK_OEM_PERIOD:
            return EKeyboardInput::Period;
        case VK_OEM_2:
            return EKeyboardInput::Slash;
        case VK_OEM_1:
            return EKeyboardInput::Semicolon;
        case VK_OEM_PLUS:
            return EKeyboardInput::Equal;
        case VK_OEM_4:
            return EKeyboardInput::LeftBracket;
        case VK_OEM_5:
            return EKeyboardInput::Backslash;
        case VK_OEM_6:
            return EKeyboardInput::RightBracket;
        case VK_OEM_3:
            return EKeyboardInput::GraveAccent;
        case VK_CAPITAL:
            return EKeyboardInput::CapsLock;
        case VK_SCROLL:
            return EKeyboardInput::ScrollLock;
        case VK_NUMLOCK:
            return EKeyboardInput::NumLock;
        case VK_SNAPSHOT:
            return EKeyboardInput::PrintScreen;
        case VK_PAUSE:
            return EKeyboardInput::Pause;

        case VK_DECIMAL:
            return EKeyboardInput::KeypadDecimal;
        case VK_DIVIDE:
            return EKeyboardInput::KeypadDivide;
        case VK_MULTIPLY:
            return EKeyboardInput::KeypadMultiply;
        case VK_SUBTRACT:
            return EKeyboardInput::KeypadSubtract;
        case VK_ADD:
            return EKeyboardInput::KeypadAdd;
        case WM_KEYPAD_RETURN:
            return EKeyboardInput::KeypadEnter;
        }
        return EKeyboardInput::None;
    }

    EMouseInput MouseInputFromPlatform(
        unsigned long      MouseCode,
        unsigned long long wParam)
    {
        switch (MouseCode)
        {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONUP:
            return EMouseInput::Left;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
            return EMouseInput::Right;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONUP:
            return EMouseInput::Middle;

        case WM_XBUTTONDOWN:
        case WM_XBUTTONDBLCLK:
        case WM_XBUTTONUP:
            return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? EMouseInput::X1 : EMouseInput::X2;
        }
        return EMouseInput::None;
    }
} // namespace Neon::Input
