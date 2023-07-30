#include <WindowPCH.hpp>
#include <Input/Data/InputMouseWheel.hpp>

namespace Neon::Input
{
    auto InputMouseWheel::Bind(
        BindType                           Type,
        InputDelegateHandler::DelegateType Delegate) -> uint64_t
    {
        return LoadHandler(Type)->Listen(std::move(Delegate));
    }

    void InputMouseWheel::Unbind(
        BindType Type,
        uint64_t Handle)
    {
        LoadHandler(Type)->Drop(Handle);
    }

    void InputMouseWheel::UnbindAll(
        BindType Type)
    {
        LoadHandler(Type)->DropAll();
    }

    size_t InputMouseWheel::GetListenersCount(
        BindType Type) const
    {
        const InputDelegateHandler* Handler = GetHandler(Type);
        return Handler ? Handler->GetListenersCount() : 0;
    }

    void InputMouseWheel::Dispatch(
        BindType   Type,
        float      Factor,
        MotionType Motion)
    {
        if (auto Handler = GetHandler(Type))
        {
            Handler->Broadcast(Factor, Motion);
        }
    }

    auto InputMouseWheel::LoadHandler(BindType Type) -> InputDelegateHandler*
    {
        std::unique_ptr<InputDelegateHandler>* Handler = nullptr;
        switch (Type)
        {
        case InputMouseWheel::BindType::Horizontal:
            Handler = &m_OnWheelHorizontal;
            break;
        case InputMouseWheel::BindType::Vertical:
            Handler = &m_OnWheelVertical;
            break;
        }

        if (!Handler->get())
        {
            *Handler = std::make_unique<InputDelegateHandler>();
        }
        return Handler->get();
    }

    auto InputMouseWheel::GetHandler(BindType Type) const -> const InputDelegateHandler*
    {
        const InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case InputMouseWheel::BindType::Horizontal:
            Handler = m_OnWheelHorizontal.get();
            break;
        case InputMouseWheel::BindType::Vertical:
            Handler = m_OnWheelVertical.get();
            break;
        }
        return Handler;
    }

    auto InputMouseWheel::GetHandler(
        BindType Type) -> InputDelegateHandler*
    {
        InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case InputMouseWheel::BindType::Horizontal:
            Handler = m_OnWheelHorizontal.get();
            break;
        case InputMouseWheel::BindType::Vertical:
            Handler = m_OnWheelVertical.get();
            break;
        }
        return Handler;
    }

    InputMouseWheelDataEvent::InputMouseWheelDataEvent(
        Ref<InputMouseWheel>        Mouse,
        float                        Factor,
        InputMouseWheel::BindType   Type,
        InputMouseWheel::MotionType Motion) :
        m_InputAxis(std::move(Mouse)),
        m_Factor(Factor),
        m_InputType(Type),
        m_Motion(Motion)
    {
    }

    void InputMouseWheelDataEvent::DispatchInput()
    {
        if (auto Axis = m_InputAxis.lock())
        {
            Axis->Dispatch(m_InputType, m_Factor, m_Motion);
        }
    }
} // namespace Neon::Input