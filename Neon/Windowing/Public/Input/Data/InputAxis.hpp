#pragma once

#include <Input/Data/InputData.hpp>
#include <Utils/Signal.hpp>

namespace Neon::Input
{
    class InputAxis
    {
        friend class InputAxisDataEvent;
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
        /// <param name="InputType"></param>
        void SetInput(
            EKeyboardInput InputType);

        /// <summary>
        /// Get the input type.
        /// </summary>
        [[nodiscard]] EKeyboardInput GetInput() const noexcept
        {
            return m_InputType;
        }

        /// <summary>
        /// Set the scale of the input axis.
        /// </summary>
        void SetScale(
            float Scale)
        {
            m_Scale = Scale;
        }

        /// <summary>
        /// Get the scale of the input axis.
        /// </summary>
        [[nodiscard]] float GetScale() const noexcept
        {
            return m_Scale;
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
        void Dispatch(BindType Type);

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
        float                                 m_Scale     = 1.f;
        EKeyboardInput                        m_InputType = EKeyboardInput::None;
        std::unique_ptr<InputDelegateHandler> m_OnActionDown, m_OnActionTick, m_OnActionUp;
    };

    class InputAxisDataEvent final
    {
    public:
        InputAxisDataEvent(
            EKeyboardInput      InputType,
            InputAxis::BindType BindType);

        /// <summary>
        /// Dispatch the input type.
        /// </summary>
        void DispatchInput(
            class InputAxisTable* Axis);

    private:
        EKeyboardInput      m_InputType;
        InputAxis::BindType m_BindType;
    };
} // namespace Neon::Input