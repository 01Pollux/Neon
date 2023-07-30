#pragma once

#include <Input/Data/InputData.hpp>
#include <Utils/Signal.hpp>

#include <Math/Vector.hpp>

namespace Neon::Input
{
    class IInputMouse
    {
        friend class InputMouseDataEvent;
        using InputDelegateHandler = Utils::Signal<const Vector2&>;

    public:
        enum class BindType : uint8_t
        {
            Click,
            DoubleClick,
            Release,
            Move
        };

        /// <summary>
        /// Set the input type.
        /// </summary>
        void SetInput(
            EMouseInput InputType)
        {
            m_InputType = InputType;
        }

        /// <summary>
        /// Get the input type.
        /// </summary>
        [[nodiscard]] EMouseInput GetInput() const noexcept
        {
            return m_InputType;
        }

        /// <summary>
        /// Bind a delegate to the input type.
        /// </summary>
        uint64_t Bind(
            BindType                           Type,
            InputDelegateHandler::DelegateType Delegate);

        /// <summary>
        /// Unbind a delegate from the input type.
        /// </summary>
        void Unbind(
            BindType Type,
            uint64_t Handle);

        /// <summary>
        /// Unbind all delegates from the input type.
        /// </summary>
        /// <param name="Type"></param>
        void UnbindAll(
            BindType Type);

        /// <summary>
        /// Get the number of listeners for the input type.
        /// </summary>
        [[nodiscard]] size_t GetListenersCount(
            BindType Type) const;

    private:
        /// <summary>
        /// Dispatch the input type.
        /// </summary>
        void Dispatch(
            BindType       Type,
            const Vector2& ScreenPosition);

        /// <summary>
        /// Get the handler for the input type.
        /// </summary>
        [[nodiscard]] InputDelegateHandler* LoadHandler(
            BindType Type);

        /// <summary>
        /// Get the handler for the input type.
        /// </summary>
        [[nodiscard]] const InputDelegateHandler* GetHandler(
            BindType Type) const;

        /// <summary>
        /// Get the handler for the input type.
        /// </summary>
        [[nodiscard]] InputDelegateHandler* GetHandler(
            BindType Type);

    private:
        std::unique_ptr<InputDelegateHandler>
            m_OnMouseClick,
            m_OnMouseDoubleClick,
            m_OnMouseRelease,
            m_OnMouseMove;
        EMouseInput m_InputType = EMouseInput::None;
    };

    class InputMouseDataEvent final
    {
    public:
        InputMouseDataEvent(
            Ref<IInputMouse>      Mouse,
            const Vector2&        MoveDelta,
            IInputMouse::BindType Type);

        void DispatchInput();

    private:
        Ref<IInputMouse>      m_InputAxis;
        Vector2               m_MoveDelta;
        IInputMouse::BindType m_InputType;
    };
} // namespace Neon::Input
