#pragma once

#include <Input/Data/InputData.hpp>
#include <Utils/Signal.hpp>

#include <Math/Vector.hpp>

namespace Neon::Input
{
    class InputMouse
    {
        friend class InputMouseDataEvent;
        /// <summary>
        /// NewPos is (0,0) if the event type is not Move.
        /// </summary>
        using InputDelegateHandler = Utils::Signal<const Vector2& /*NewPos*/>;

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
        /// Enable or disable system input.
        /// </summary>
        void Requires(
            EKeyboardInput SysInput,
            bool           State = true);

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
            BindType                Type,
            const InputSysKeyState& SysKeyState,
            const Vector2&          ScreenPosition);

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

        InputSysKeyState m_RequiredSysInputs;
        EMouseInput      m_InputType = EMouseInput::None;
    };

    class InputMouseDataEvent final
    {
    public:
        InputMouseDataEvent(
            const Vector2&       NewPos,
            InputSysKeyState     KeyState,
            EMouseInput          InputType,
            InputMouse::BindType BindType);

        /// <summary>
        /// Dispatch the input type.
        /// </summary>
        void DispatchInput(
            class InputMouseTable* Table);

    private:
        Vector2              m_NewPos;
        InputSysKeyState     m_KeyState;
        EMouseInput          m_InputType;
        InputMouse::BindType m_BindType;
    };
} // namespace Neon::Input
