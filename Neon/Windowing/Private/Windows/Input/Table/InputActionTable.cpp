#include <WindowPCH.hpp>
#include <Input/Data/InputAction.hpp>

#include <Private/Windows/Input/Table/InputActionTable.hpp>
#include <Private/Windows/Input/Types.hpp>

namespace Neon::Input
{
    Ptr<IInputActionTable> IInputActionTable::Create()
    {
        return Ptr<IInputActionTable>{ NEON_NEW InputActionTableImpl };
    }

    bool InputActionTableImpl::PushMessage(
        InputEventQueue& EventQueue,
        UINT             Message,
        WPARAM           wParam,
        LPARAM           lParam)
    {
        if (m_InputDatas.empty())
        {
            return false;
        }

        InputAction::BindType Type;
        switch (Message)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            Type = !(HIWORD(lParam) & KF_REPEAT) ? InputAction::BindType::Press : InputAction::BindType::Tick;
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            Type = InputAction::BindType::Release;
            break;
        default:
            return false;
        }

        EKeyboardInput InputType = KeyboardInputFromPlatform(wParam, lParam);
        if (InputType == EKeyboardInput::None)
        {
            return false;
        }

        bool Processed = false;
        m_SysKeyState.SetSysControlState(InputType, Type != InputAction::BindType::Release);
        for (const auto& Input : m_InputDatas)
        {
            if (Input->GetListenersCount(Type) && Input->GetInput() == InputType)
            {
                EventQueue.QueueData<InputActionDataEvent>(Input, m_SysKeyState, Type);
                Processed = true;
            }
        }
        return Processed;
    }
} // namespace Neon::Input