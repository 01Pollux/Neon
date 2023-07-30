#include <WindowPCH.hpp>
#include <Input/Data/InputAxis.hpp>

#include <Private/Windows/Input/Table/InputAxisTable.hpp>
#include <Private/Windows/Input/Types.hpp>

namespace Neon::Input
{
    Ptr<IInputAxisTable> IInputAxisTable::Create()
    {
        return Ptr<IInputAxisTable>{ NEON_NEW InputAxisTableImpl };
    }

    bool InputAxisTableImpl::PushMessage(
        InputEventQueue& EventQueue,
        UINT             Message,
        WPARAM           wParam,
        LPARAM           lParam)
    {
        if (m_InputDatas.empty())
        {
            return false;
        }

        IInputAxis::BindType Type;
        switch (Message)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            Type = !(HIWORD(lParam) & KF_REPEAT) ? IInputAxis::BindType::Press : IInputAxis::BindType::Tick;
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            Type = IInputAxis::BindType::Release;
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
        for (const auto& Input : m_InputDatas)
        {
            if (Input->GetListenersCount(Type) && Input->GetInput() == InputType)
            {
                EventQueue.QueueData<InputAxisDataEvent>(Input, Type);
                Processed = true;
            }
        }
        return Processed;
    }
} // namespace Neon::Input