#pragma once

#include <Input/Data/InputData.hpp>
#include <Utils/Signal.hpp>

namespace Neon::Input
{
    class InputAction
    {
        friend class InputActionDataEvent;
        using InputDelegateHandler = Utils::Signal<>;

    public:
        enum class BindType : uint8_t
        {
            Press,
            Release,
            Tick
        };

        /// <summary>
        /// Set the input type.
        /// </summary>
        void SetInput(
            EKeyboardInput InputType)
        {
            m_InputType = InputType;
        }

        /// <summary>
        /// Get the input type.
        /// </summary>
        [[nodiscard]] EKeyboardInput GetInput() const noexcept
        {
            return m_InputType;
        }

        /// <summary>
        /// Enable or disable system input.
        /// </summary>
        void Requires(
            EKeyboardInput SysInput,
            bool           State);

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
            const InputSysKeyState& SysKeyState);

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
        EKeyboardInput                        m_InputType = EKeyboardInput::None;
        InputSysKeyState                      m_RequiredSysInputs;
        std::unique_ptr<InputDelegateHandler> m_OnActionDown, m_OnActionTick, m_OnActionUp;
    };

    class InputActionDataEvent final
    {
    public:
        InputActionDataEvent(
            Ref<InputAction>      Action,
            InputSysKeyState      SysKeyState,
            InputAction::BindType Type);

        void DispatchInput();

    private:
        Ref<InputAction>      m_InputAction;
        InputSysKeyState      m_SysKeyState;
        InputAction::BindType m_InputType;
    };
} // namespace Neon::Input