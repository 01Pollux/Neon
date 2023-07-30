#include <WindowPCH.hpp>
#include <Input/Data/InputMouseWheel.hpp>

#include <Private/Windows/Input/Table/InputMouseWheelTable.hpp>
#include <Private/Windows/Input/Types.hpp>

#include <windowsx.h>

namespace Neon::Input
{
    Ptr<IInputMouseWheelTable> IInputMouseWheelTable::Create()
    {
        return Ptr<IInputMouseWheelTable>{ NEON_NEW InputMouseWheelTableImpl };
    }

    bool InputMouseWheelTableImpl::PushMessage(
        InputEventQueue& EventQueue,
        UINT             Message,
        WPARAM           wParam,
        LPARAM           lParam)
    {
        if (m_InputDatas.empty())
        {
            return false;
        }

        IInputMouseWheel::BindType   Type;
        IInputMouseWheel::MotionType Motion;

        switch (Message)
        {
        case WM_MOUSEHWHEEL:
        {
            Type = IInputMouseWheel::BindType::Horizontal;

            short MotionVal = short(HIWORD(wParam));
            Motion          = MotionVal > 0 ? IInputMouseWheel::MotionType::Right : IInputMouseWheel::MotionType::Left;
            break;
        }

        case WM_MOUSEWHEEL:
        {
            Type = IInputMouseWheel::BindType::Vertical;

            short MotionVal = short(HIWORD(wParam));
            Motion          = MotionVal > 0 ? IInputMouseWheel::MotionType::Up : IInputMouseWheel::MotionType::Down;
            break;
        }

        default:
            return false;
        }

        float Factor    = GET_WHEEL_DELTA_WPARAM(wParam) / float(WHEEL_DELTA);
        bool  Processed = false;

        for (const auto& Input : m_InputDatas)
        {
            if (Input->GetListenersCount(Type))
            {
                EventQueue.QueueData<InputMouseWheelDataEvent>(Input, Factor, Type, Motion);
                Processed = true;
            }
        }

        return Processed;
    }
} // namespace Neon::Input
