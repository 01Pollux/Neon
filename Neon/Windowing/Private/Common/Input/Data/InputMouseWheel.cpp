#include <WindowPCH.hpp>
#include <Input/Data/InputMouseWheel.hpp>

namespace Neon::Input
{
    auto IInputMouseWheel::Bind(
        BindType                           Type,
        InputDelegateHandler::DelegateType Delegate) -> uint64_t
    {
        return LoadHandler(Type)->Listen(std::move(Delegate));
    }

    void IInputMouseWheel::Unbind(
        BindType Type,
        uint64_t Handle)
    {
        LoadHandler(Type)->Drop(Handle);
    }

    void IInputMouseWheel::UnbindAll(
        BindType Type)
    {
        LoadHandler(Type)->DropAll();
    }

    size_t IInputMouseWheel::GetListenersCount(
        BindType Type) const
    {
        const InputDelegateHandler* Handler = GetHandler(Type);
        return Handler ? Handler->GetListenersCount() : 0;
    }

    void IInputMouseWheel::Dispatch(
        BindType   Type,
        float      Factor,
        MotionType Motion)
    {
        if (auto Handler = GetHandler(Type))
        {
            Handler->Broadcast(Factor, Motion);
        }
    }

    auto IInputMouseWheel::LoadHandler(BindType Type) -> InputDelegateHandler*
    {
        std::unique_ptr<InputDelegateHandler>* Handler = nullptr;
        switch (Type)
        {
        case IInputMouseWheel::BindType::Horizontal:
            Handler = &m_OnWheelHorizontal;
            break;
        case IInputMouseWheel::BindType::Vertical:
            Handler = &m_OnWheelVertical;
            break;
        }

        if (!Handler->get())
        {
            *Handler = std::make_unique<InputDelegateHandler>();
        }
        return Handler->get();
    }

    auto IInputMouseWheel::GetHandler(BindType Type) const -> const InputDelegateHandler*
    {
        const InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case IInputMouseWheel::BindType::Horizontal:
            Handler = m_OnWheelHorizontal.get();
            break;
        case IInputMouseWheel::BindType::Vertical:
            Handler = m_OnWheelVertical.get();
            break;
        }
        return Handler;
    }

    auto IInputMouseWheel::GetHandler(
        BindType Type) -> InputDelegateHandler*
    {
        InputDelegateHandler* Handler = nullptr;
        switch (Type)
        {
        case IInputMouseWheel::BindType::Horizontal:
            Handler = m_OnWheelHorizontal.get();
            break;
        case IInputMouseWheel::BindType::Vertical:
            Handler = m_OnWheelVertical.get();
            break;
        }
        return Handler;
    }

    InputMouseWheelDataEvent::InputMouseWheelDataEvent(
        Ref<IInputMouseWheel>        Mouse,
        float                        Factor,
        IInputMouseWheel::BindType   Type,
        IInputMouseWheel::MotionType Motion) :
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