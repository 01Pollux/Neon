#include <WindowPCH.hpp>
#include <Input/Data/InputAxis.hpp>
#include <Input/Table/InputAxisTable.hpp>

namespace Neon::Input
{
    uint64_t InputAxis::Bind(
        BindType                           Type,
        InputDelegateHandler::DelegateType Delegate)
    {
        return LoadHandler(Type)->Listen(std::move(Delegate));
    }

    void InputAxis::Unbind(
        BindType Type,
        uint64_t Handle)
    {
        LoadHandler(Type)->Drop(Handle);
    }

    void InputAxis::UnbindAll(
        BindType Type)
    {
        LoadHandler(Type)->DropAll();
    }

    size_t InputAxis::GetListenersCount(
        BindType Type) const
    {
        const InputDelegateHandler* Handler = GetHandler(Type);
        return Handler ? Handler->GetListenersCount() : 0;
    }

    void InputAxis::Dispatch(
        BindType Type)
    {
        if (auto Listener = GetHandler(Type))
        {
            Listener->Broadcast();
        }
    }

    auto InputAxis::LoadHandler(
        BindType Type) -> InputDelegateHandler*
    {
        std::unique_ptr<InputDelegateHandler>* Handler = nullptr;
        switch (Type)
        {
        case InputAxis::BindType::Press:
            Handler = &m_OnActionDown;
            break;
        case InputAxis::BindType::Release:
            Handler = &m_OnActionUp;
            break;
        case InputAxis::BindType::Tick:
            Handler = &m_OnActionTick;
            break;
        }

        if (!Handler->get())
        {
            *Handler = std::make_unique<InputDelegateHandler>();
        }
        return Handler->get();
    }

    auto InputAxis::GetHandler(
        BindType Type) const -> const InputDelegateHandler*
    {
        const InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case InputAxis::BindType::Press:
            Handler = m_OnActionDown.get();
            break;
        case InputAxis::BindType::Release:
            Handler = m_OnActionUp.get();
            break;
        case InputAxis::BindType::Tick:
            Handler = m_OnActionTick.get();
            break;
        }
        return Handler;
    }

    auto InputAxis::GetHandler(
        BindType Type) -> InputDelegateHandler*
    {
        InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case InputAxis::BindType::Press:
            Handler = m_OnActionDown.get();
            break;
        case InputAxis::BindType::Release:
            Handler = m_OnActionUp.get();
            break;
        case InputAxis::BindType::Tick:
            Handler = m_OnActionTick.get();
            break;
        }
        return Handler;
    }

    InputAxisDataEvent::InputAxisDataEvent(
        EKeyboardInput      InputType,
        InputAxis::BindType BindType) :
        m_InputType(InputType),
        m_BindType(BindType)
    {
    }

    void InputAxisDataEvent::DispatchInput(
        IInputAxisTable* Table)
    {
        for (auto& Axis : Table->GetAxis())
        {
            if (Axis->m_InputType == m_InputType)
            {
                Axis->Dispatch(m_BindType);
            }
        }
    }
} // namespace Neon::Input
