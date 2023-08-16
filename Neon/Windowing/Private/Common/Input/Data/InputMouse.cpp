#include <WindowPCH.hpp>
#include <Input/Data/InputMouse.hpp>
#include <Input/Table/InputMouseTable.hpp>

namespace Neon::Input
{
    void InputMouse::Requires(
        EKeyboardInput SysInput,
        bool           State)
    {
        m_RequiredSysInputs.SetSysControlState(SysInput, State);
    }

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
        BindType                Type,
        const InputSysKeyState& SysKeyState,
        const Vector2&          MoveDelta)
    {
        if (m_RequiredSysInputs.ContainsAll(SysKeyState))
        {
            if (auto Handler = GetHandler(Type))
            {
                Handler->Broadcast(MoveDelta);
            }
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
        const Vector2&       NewPos,
        InputSysKeyState     KeyState,
        EMouseInput          InputType,
        InputMouse::BindType BindType) :
        m_NewPos(NewPos),
        m_KeyState(KeyState),
        m_InputType(InputType),
        m_BindType(BindType)
    {
    }

    void InputMouseDataEvent::DispatchInput(
        InputMouseTable* Table)
    {
        for (auto& Mouse : Table->GetMouses())
        {
            if (Mouse->m_InputType == m_InputType)
            {
                Mouse->Dispatch(m_BindType, m_KeyState, m_NewPos);
            }
        }
    }
} // namespace Neon::Input
