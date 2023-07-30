#include <WindowPCH.hpp>
#include <Input/Data/InputAction.hpp>

#include <Log/Logger.hpp>

namespace Neon::Input
{
    void InputAction::Requires(
        EKeyboardInput SysInput,
        bool           State)
    {
        NEON_ASSERT(SysInput >= EKeyboardInput::_Begin_SysControl && SysInput <= EKeyboardInput::_End_SysContrl);
        m_RequiredSysInputs.SetSysControlState(SysInput, State);
    }

    uint64_t InputAction::Bind(
        BindType                           Type,
        InputDelegateHandler::DelegateType Delegate)
    {
        return LoadHandler(Type)->Listen(Delegate);
    }

    void InputAction::Unbind(
        BindType Type,
        uint64_t Handle)
    {
        LoadHandler(Type)->Drop(Handle);
    }

    void InputAction::UnbindAll(
        BindType Type)
    {
        LoadHandler(Type)->DropAll();
    }

    size_t InputAction::GetListenersCount(
        BindType Type) const
    {
        auto Handler = GetHandler(Type);
        return Handler ? Handler->GetListenersCount() : 0;
    }

    void InputAction::Dispatch(
        BindType                Type,
        const InputSysKeyState& SysKeyState)
    {
        if (m_RequiredSysInputs.IsEmpty() || m_RequiredSysInputs.ContainsAll(SysKeyState))
        {
            GetHandler(Type)->Broadcast();
        }
    }

    auto InputAction::LoadHandler(BindType Type) -> InputDelegateHandler*
    {
        std::unique_ptr<InputDelegateHandler>* Handler = nullptr;
        switch (Type)
        {
        case InputAction::BindType::Press:
            Handler = &m_OnActionDown;
            break;
        case InputAction::BindType::Release:
            Handler = &m_OnActionUp;
            break;
        case InputAction::BindType::Tick:
            Handler = &m_OnActionTick;
            break;
        }

        if (!Handler->get())
        {
            *Handler = std::make_unique<InputDelegateHandler>();
        }
        return Handler->get();
    }

    auto InputAction::GetHandler(
        BindType Type) const -> const InputDelegateHandler*
    {
        const InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case InputAction::BindType::Press:
            Handler = m_OnActionDown.get();
            break;
        case InputAction::BindType::Release:
            Handler = m_OnActionUp.get();
            break;
        case InputAction::BindType::Tick:
            Handler = m_OnActionTick.get();
            break;
        }
        return Handler;
    }

    auto InputAction::GetHandler(
        BindType Type) -> InputDelegateHandler*
    {
        InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case InputAction::BindType::Press:
            Handler = m_OnActionDown.get();
            break;
        case InputAction::BindType::Release:
            Handler = m_OnActionUp.get();
            break;
        case InputAction::BindType::Tick:
            Handler = m_OnActionTick.get();
            break;
        }
        return Handler;
    }

    InputActionDataEvent::InputActionDataEvent(
        Ref<InputAction>      Action,
        InputSysKeyState      SysKeyState,
        InputAction::BindType Type) :
        m_InputAction(std::move(Action)),
        m_SysKeyState(SysKeyState),
        m_InputType(Type)
    {
    }

    void InputActionDataEvent::DispatchInput()
    {
        if (auto Action = m_InputAction.lock())
        {
            Action->Dispatch(m_InputType, m_SysKeyState);
        }
    }
} // namespace Neon::Input
