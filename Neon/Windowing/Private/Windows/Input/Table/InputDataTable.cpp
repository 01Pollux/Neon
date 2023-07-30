#include <WindowPCH.hpp>
#include <Private/Windows/Input/Table/InputDataTable.hpp>

#include <Private/Windows/Input/Table/InputActionTable.hpp>
#include <Private/Windows/Input/Table/InputAxisTable.hpp>
#include <Private/Windows/Input/Table/InputMouseTable.hpp>
#include <Private/Windows/Input/Table/InputMouseWheelTable.hpp>

namespace Neon::Input
{
    Ptr<IInputDataTable> IInputDataTable::Create()
    {
        return Ptr<IInputDataTable>{ NEON_NEW InputDataTableImpl };
    }

    LRESULT InputDataTableImpl::PushMessage(
        HWND   WindowHandle,
        UINT   Message,
        WPARAM wParam,
        LPARAM lParam,
        bool&  Handled)
    {
        for (auto& [ActionName, Action] : m_InputActionMap)
        {
            if (Action->IsEnabled())
            {
                Handled |= static_cast<InputActionTableImpl*>(Action.get())->PushMessage(m_EventQueue, Message, wParam, lParam);
            }
        }
        for (auto& [ActionName, Axis] : m_InputAxisMap)
        {
            if (Axis->IsEnabled())
            {
                Handled |= static_cast<InputAxisTableImpl*>(Axis.get())->PushMessage(m_EventQueue, Message, wParam, lParam);
            }
        }
        for (auto& [ActionName, Mouse] : m_InputMouseMap)
        {
            if (Mouse->IsEnabled())
            {
                Handled |= static_cast<InputMouseTableImpl*>(Mouse.get())->PushMessage(m_EventQueue, WindowHandle, Message, wParam, lParam);
            }
        }
        for (auto& [ActionName, Mouse] : m_InputMouseWheelMap)
        {
            if (Mouse->IsEnabled())
            {
                Handled |= static_cast<InputMouseWheelTableImpl*>(Mouse.get())->PushMessage(m_EventQueue, Message, wParam, lParam);
            }
        }
        return 0;
    }
} // namespace Neon::Input