#include <WindowPCH.hpp>
#include <Input/Table/InputDataTable.hpp>

namespace Neon::Input
{
    void IInputDataTable::ProcessInputs()
    {
        m_EventQueue.Dispatch();
    }

    Ref<IInputActionTable> IInputDataTable::LoadActionTable(
        const StringU8& ActionName)
    {
        auto& Action = m_InputActionMap[ActionName];
        if (!Action)
        {
            Action = IInputActionTable::Create();
        }
        return Action;
    }

    Ref<IInputActionTable> IInputDataTable::GetActionTable(
        const StringU8& ActionName) const
    {
        auto Iter = m_InputActionMap.find(ActionName);
        return Iter == m_InputActionMap.end() ? nullptr : Iter->second;
    }

    void IInputDataTable::RemoveActionTable(
        const StringU8& ActionName)
    {
        m_InputActionMap.erase(ActionName);
    }

    //

    Ref<IInputAxisTable> IInputDataTable::LoadAxisTable(
        const StringU8& AxisName)
    {
        auto& Axis = m_InputAxisMap[AxisName];
        if (!Axis)
        {
            Axis = IInputAxisTable::Create();
        }
        return Axis;
    }

    Ref<IInputAxisTable> IInputDataTable::GetAxisTable(
        const StringU8& AxisName) const
    {
        auto Iter = m_InputAxisMap.find(AxisName);
        return Iter == m_InputAxisMap.end() ? nullptr : Iter->second;
    }

    void IInputDataTable::RemoveAxisTable(
        const StringU8& AxisName)
    {
        m_InputAxisMap.erase(AxisName);
    }

    //

    Ref<IInputMouseTable> IInputDataTable::LoadMouseTable(
        const StringU8& MouseName)
    {
        auto& Mouse = m_InputMouseMap[MouseName];
        if (!Mouse)
        {
            Mouse = IInputMouseTable::Create();
        }
        return Mouse;
    }

    Ref<IInputMouseTable> IInputDataTable::GetMouseTable(
        const StringU8& MouseName) const
    {
        auto Iter = m_InputMouseMap.find(MouseName);
        return Iter == m_InputMouseMap.end() ? nullptr : Iter->second;
    }

    void IInputDataTable::RemoveMouseTable(
        const StringU8& MouseName)
    {
        m_InputMouseMap.erase(MouseName);
    }

    //

    Ref<IInputMouseWheelTable> IInputDataTable::LoadMouseWheelTable(
        const StringU8& MouseWheelName)
    {
        auto& MouseWheel = m_InputMouseWheelMap[MouseWheelName];
        if (!MouseWheel)
        {
            MouseWheel = IInputMouseWheelTable::Create();
        }
        return MouseWheel;
    }

    Ref<IInputMouseWheelTable> IInputDataTable::GetMouseWheelTable(
        const StringU8& MouseWheelName) const
    {
        auto Iter = m_InputMouseWheelMap.find(MouseWheelName);
        return Iter == m_InputMouseWheelMap.end() ? nullptr : Iter->second;
    }

    void IInputDataTable::RemoveMouseWheelTable(
        const StringU8& MouseWheelName)
    {
        m_InputMouseWheelMap.erase(MouseWheelName);
    }

    void InputEventQueue::Dispatch()
    {
        auto DispatchAndClearPool = [this](auto& Pool)
        {
            for (auto& InputData : Pool)
            {
                InputData.DispatchInput();
            }
            Pool.clear();
        };

        DispatchAndClearPool(GetPool<InputActionDataEvent>());
        DispatchAndClearPool(GetPool<InputAxisDataEvent>());
        DispatchAndClearPool(GetPool<InputMouseDataEvent>());
        DispatchAndClearPool(GetPool<InputMouseWheelDataEvent>());
    }
} // namespace Neon::Input