#include <WindowPCH.hpp>
#include <Input/Data/InputMouse.hpp>

namespace Neon::Input
{
    uint64_t InputMouse::Bind(
        BindType                           Type,
        InputDelegateHandler::DelegateType Delegate)
    {
        return LoadHandler(Type)->Listen(std::move(Delegate));
    }

    void InputMouse::Unbind(
        BindType Type,
        uint64_t Handle)
    {
        return LoadHandler(Type)->Drop(Handle);
    }

    void InputMouse::UnbindAll(
        BindType Type)
    {
        LoadHandler(Type)->DropAll();
    }

    size_t InputMouse::GetListenersCount(
        BindType Type) const
    {
        const InputDelegateHandler* Handler = GetHandler(Type);
        return Handler ? Handler->GetListenersCount() : 0;
    }

    void InputMouse::Dispatch(
        BindType       Type,
        const Vector2& MoveDelta)
    {
        if (auto Handler = GetHandler(Type))
        {
            Handler->Broadcast(MoveDelta);
        }
    }

    auto InputMouse::LoadHandler(
        BindType Type) -> InputDelegateHandler*
    {
        std::unique_ptr<InputDelegateHandler>* Handler = nullptr;
        switch (Type)
        {
        case InputMouse::BindType::Click:
            Handler = &m_OnMouseClick;
            break;
        case InputMouse::BindType::DoubleClick:
            Handler = &m_OnMouseDoubleClick;
            break;
        case InputMouse::BindType::Release:
            Handler = &m_OnMouseRelease;
            break;
        case InputMouse::BindType::Move:
            Handler = &m_OnMouseMove;
            break;
        }

        if (!Handler->get())
        {
            *Handler = std::make_unique<InputDelegateHandler>();
        }
        return Handler->get();
    }

    auto InputMouse::GetHandler(
        BindType Type) const -> const InputDelegateHandler*
    {
        const InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case InputMouse::BindType::Click:
            Handler = m_OnMouseClick.get();
            break;
        case InputMouse::BindType::DoubleClick:
            Handler = m_OnMouseDoubleClick.get();
            break;
        case InputMouse::BindType::Release:
            Handler = m_OnMouseRelease.get();
            break;
        case InputMouse::BindType::Move:
            Handler = m_OnMouseMove.get();
            break;
        }
        return Handler;
    }

    auto InputMouse::GetHandler(
        BindType Type) -> InputDelegateHandler*
    {
        InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case InputMouse::BindType::Click:
            Handler = m_OnMouseClick.get();
            break;
        case InputMouse::BindType::DoubleClick:
            Handler = m_OnMouseDoubleClick.get();
            break;
        case InputMouse::BindType::Release:
            Handler = m_OnMouseRelease.get();
            break;
        case InputMouse::BindType::Move:
            Handler = m_OnMouseMove.get();
            break;
        }
        return Handler;
    }

    InputMouseDataEvent::InputMouseDataEvent(
        Ref<InputMouse>      Mouse,
        const Vector2&        MoveDelta,
        InputMouse::BindType Type) :
        m_InputAxis(std::move(Mouse)),
        m_MoveDelta(MoveDelta),
        m_InputType(Type)
    {
    }

    void InputMouseDataEvent::DispatchInput()
    {
        if (auto Mouse = m_InputAxis.lock())
        {
            Mouse->Dispatch(m_InputType, m_MoveDelta);
        }
    }
} // namespace Neon::Input
