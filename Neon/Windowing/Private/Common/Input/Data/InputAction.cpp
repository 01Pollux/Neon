#include <WindowPCH.hpp>
#include <Input/Data/InputAction.hpp>

#include <Log/Logger.hpp>

namespace Neon::Input
{
    void IInputAction::Requires(
        EKeyboardInput SysInput,
        bool           State)
    {
        NEON_ASSERT(SysInput >= EKeyboardInput::_Begin_SysControl && SysInput <= EKeyboardInput::_End_SysContrl);
        m_RequiredSysInputs.SetSysControlState(SysInput, State);
    }

    uint64_t IInputAction::Bind(
        BindType                           Type,
        InputDelegateHandler::DelegateType Delegate)
    {
        return LoadHandler(Type)->Listen(Delegate);
    }

    void IInputAction::Unbind(
        BindType Type,
        uint64_t Handle)
    {
        LoadHandler(Type)->Drop(Handle);
    }

    void IInputAction::UnbindAll(
        BindType Type)
    {
        LoadHandler(Type)->DropAll();
    }

    size_t IInputAction::GetListenersCount(
        BindType Type) const
    {
        return GetHandler(Type)->GetListenersCount();
    }

    void IInputAction::Dispatch(
        BindType                Type,
        const InputSysKeyState& SysKeyState)
    {
        if (m_RequiredSysInputs.IsEmpty() || m_RequiredSysInputs.ContainsAll(SysKeyState))
        {
            GetHandler(Type)->Broadcast();
        }
    }

    auto IInputAction::LoadHandler(BindType Type) -> InputDelegateHandler*
    {
        std::unique_ptr<InputDelegateHandler>* Handler = nullptr;
        switch (Type)
        {
        case IInputAction::BindType::Press:
            Handler = &m_OnActionDown;
            break;
        case IInputAction::BindType::Release:
            Handler = &m_OnActionUp;
            break;
        case IInputAction::BindType::Tick:
            Handler = &m_OnActionTick;
            break;
        }

        if (!Handler->get())
        {
            *Handler = std::make_unique<InputDelegateHandler>();
        }
        return Handler->get();
    }

    auto IInputAction::GetHandler(
        BindType Type) const -> const InputDelegateHandler*
    {
        const InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case IInputAction::BindType::Press:
            Handler = m_OnActionDown.get();
            break;
        case IInputAction::BindType::Release:
            Handler = m_OnActionUp.get();
            break;
        case IInputAction::BindType::Tick:
            Handler = m_OnActionTick.get();
            break;
        }
        return Handler;
    }

    auto IInputAction::GetHandler(
        BindType Type) -> InputDelegateHandler*
    {
        InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case IInputAction::BindType::Press:
            Handler = m_OnActionDown.get();
            break;
        case IInputAction::BindType::Release:
            Handler = m_OnActionUp.get();
            break;
        case IInputAction::BindType::Tick:
            Handler = m_OnActionTick.get();
            break;
        }
        return Handler;
    }

    InputActionDataEvent::InputActionDataEvent(
        Ref<IInputAction>      Action,
        InputSysKeyState       SysKeyState,
        IInputAction::BindType Type) :
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
