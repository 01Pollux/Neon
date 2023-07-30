#include <WindowPCH.hpp>
#include <Input/Data/InputMouse.hpp>

#include <Private/Windows/Input/Table/InputMouseTable.hpp>
#include <Private/Windows/Input/Types.hpp>

#include <windowsx.h>
#include <Log/Logger.hpp>

namespace Neon::Input
{
    Ptr<IInputMouseTable> IInputMouseTable::Create()
    {
        return Ptr<IInputMouseTable>{ NEON_NEW InputMouseTableImpl };
    }

    bool InputMouseTableImpl::PushMessage(
        InputEventQueue& EventQueue,
        HWND             WindowHandle,
        UINT             Message,
        WPARAM           wParam,
        LPARAM           lParam)
    {
        if (m_InputDatas.empty())
        {
            return false;
        }

        EMouseInput           InputType;
        IInputMouse::BindType Type;
        Vector2               MoveDelta;

        if (!ReadMouseInput(
                WindowHandle,
                Message,
                wParam,
                lParam,
                MoveDelta,
                InputType,
                Type))
        {
            return false;
        }

        bool Processed = false;
        for (const auto& Input : m_InputDatas)
        {
            if (Input->GetListenersCount(Type) && Input->GetInput() == InputType)
            {
                EventQueue.QueueData<InputMouseDataEvent>(Input, MoveDelta, Type);
            }
        }

        return Processed;
    }

    bool InputMouseTableImpl::ReadMouseInput(
        HWND                   WindowHandle,
        UINT                   Message,
        WPARAM                 wParam,
        LPARAM                 lParam,
        Vector2&               MoveDelta,
        EMouseInput&           InputType,
        IInputMouse::BindType& Type)
    {
        switch (Message)
        {
        case WM_INPUT:
        {
            UINT InputDataSize = 0;
            GetRawInputData(HRAWINPUT(lParam), RID_INPUT, nullptr, &InputDataSize, sizeof(RAWINPUTHEADER));

            m_RawInputPool.resize(InputDataSize);
            NEON_ASSERT(GetRawInputData(HRAWINPUT(lParam), RID_INPUT, m_RawInputPool.data(), &InputDataSize, sizeof(RAWINPUTHEADER)) == InputDataSize);

            PRAWINPUT InputData = PRAWINPUT(m_RawInputPool.data());
            RAWMOUSE* MouseData = &InputData->data.mouse;

            if (InputData->header.dwType == RIM_TYPEMOUSE)
            {
                if (MouseData->usFlags & MOUSE_MOVE_ABSOLUTE)
                {
                    bool isVirtualDesktop = MouseData->usFlags & MOUSE_VIRTUAL_DESKTOP;

                    int Width  = GetSystemMetrics(isVirtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
                    int Height = GetSystemMetrics(isVirtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);

                    POINT Point{};
                    Point.x = int((MouseData->lLastX / 65535.0f) * Width);
                    Point.y = int((MouseData->lLastY / 65535.0f) * Height);

                    ScreenToClient(WindowHandle, &Point);

                    MoveDelta = { float(Point.x - m_OldMousePosition.x), float(Point.y - m_OldMousePosition.y) };

                    m_OldMousePosition = Point;
                }
                else if (MouseData->lLastX != 0 || MouseData->lLastY != 0)
                {
                    MoveDelta = { float(MouseData->lLastX), float(MouseData->lLastY) };
                }

                if (MoveDelta != Vec::Zero<Vector2>)
                {
                    InputType = EMouseInput::None;
                    Type      = IInputMouse::BindType::Move;
                    return true;
                }
            }

            break;
        }

        case WM_LBUTTONDBLCLK:
            InputType = EMouseInput::Left;
            Type      = IInputMouse::BindType::DoubleClick;
            return true;

        case WM_LBUTTONDOWN:
            InputType = EMouseInput::Left;
            Type      = IInputMouse::BindType::Click;
            return true;

        case WM_LBUTTONUP:
            InputType = EMouseInput::Left;
            Type      = IInputMouse::BindType::Release;
            return true;

        case WM_RBUTTONDBLCLK:
            InputType = EMouseInput::Right;
            Type      = IInputMouse::BindType::DoubleClick;
            return true;

        case WM_RBUTTONDOWN:
            InputType = EMouseInput::Right;
            Type      = IInputMouse::BindType::Click;
            return true;

        case WM_RBUTTONUP:
            InputType = EMouseInput::Right;
            Type      = IInputMouse::BindType::Release;
            return true;

        case WM_MBUTTONDBLCLK:
            InputType = EMouseInput::Middle;
            Type      = IInputMouse::BindType::DoubleClick;
            return true;

        case WM_MBUTTONDOWN:
            InputType = EMouseInput::Middle;
            Type      = IInputMouse::BindType::Click;
            return true;

        case WM_MBUTTONUP:
            InputType = EMouseInput::Middle;
            Type      = IInputMouse::BindType::Release;
            return true;

        case WM_XBUTTONDBLCLK:
            InputType = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? EMouseInput::X1 : EMouseInput::X2;
            Type      = IInputMouse::BindType::DoubleClick;
            return true;

        case WM_XBUTTONDOWN:
            InputType = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? EMouseInput::X1 : EMouseInput::X2;
            Type      = IInputMouse::BindType::Click;
            return true;

        case WM_XBUTTONUP:
            InputType = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? EMouseInput::X1 : EMouseInput::X2;
            Type      = IInputMouse::BindType::Release;
            return true;
        }
        return false;
    }
} // namespace Neon::Input
