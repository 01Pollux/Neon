#include <WindowPCH.hpp>
#include <Input/Data/InputAxis.hpp>

namespace Neon::Input
{
    uint64_t IInputAxis::Bind(
        BindType                           Type,
        InputDelegateHandler::DelegateType Delegate)
    {
        return LoadHandler(Type)->Listen(std::move(Delegate));
    }

    void IInputAxis::Unbind(
        BindType Type,
        uint64_t Handle)
    {
        LoadHandler(Type)->Drop(Handle);
    }

    void IInputAxis::UnbindAll(
        BindType Type)
    {
        LoadHandler(Type)->DropAll();
    }

    size_t IInputAxis::GetListenersCount(
        BindType Type) const
    {
        const InputDelegateHandler* Handler = GetHandler(Type);
        return Handler ? Handler->GetListenersCount() : 0;
    }

    void IInputAxis::Dispatch(
        BindType Type)
    {
        if (auto Listener = GetHandler(Type))
        {
            Listener->Broadcast();
        }
    }

    auto IInputAxis::LoadHandler(
        BindType Type) -> InputDelegateHandler*
    {
        std::unique_ptr<InputDelegateHandler>* Handler = nullptr;
        switch (Type)
        {
        case IInputAxis::BindType::Press:
            Handler = &m_OnActionDown;
            break;
        case IInputAxis::BindType::Release:
            Handler = &m_OnActionUp;
            break;
        case IInputAxis::BindType::Tick:
            Handler = &m_OnActionTick;
            break;
        }

        if (!Handler->get())
        {
            *Handler = std::make_unique<InputDelegateHandler>();
        }
        return Handler->get();
    }

    auto IInputAxis::GetHandler(
        BindType Type) const -> const InputDelegateHandler*
    {
        const InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case IInputAxis::BindType::Press:
            Handler = m_OnActionDown.get();
            break;
        case IInputAxis::BindType::Release:
            Handler = m_OnActionUp.get();
            break;
        case IInputAxis::BindType::Tick:
            Handler = m_OnActionTick.get();
            break;
        }
        return Handler;
    }

    auto IInputAxis::GetHandler(
        BindType Type) -> InputDelegateHandler*
    {
        InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case IInputAxis::BindType::Press:
            Handler = m_OnActionDown.get();
            break;
        case IInputAxis::BindType::Release:
            Handler = m_OnActionUp.get();
            break;
        case IInputAxis::BindType::Tick:
            Handler = m_OnActionTick.get();
            break;
        }
        return Handler;
    }

    InputAxisDataEvent::InputAxisDataEvent(
        Ref<IInputAxis>      Axis,
        IInputAxis::BindType Type) :
        m_InputAxis(std::move(Axis)),
        m_InputType(Type)
    {
    }

    void InputAxisDataEvent::DispatchInput()
    {
        if (auto Axis = m_InputAxis.lock())
        {
            Axis->Dispatch(m_InputType);
        }
    }
} // namespace Neon::Input
