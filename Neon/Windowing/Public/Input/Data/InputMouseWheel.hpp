#pragma once

#include <Input/Data/InputData.hpp>
#include <Utils/Signal.hpp>

namespace Neon::Input
{
    class InputMouseWheel
    {
    public:
        enum class MotionType : uint8_t
        {
            Up,
            Down,
            Left,
            Right
        };

        enum class BindType : uint8_t
        {
            Horizontal,
            Vertical
        };

        friend class InputMouseWheelDataEvent;
        using InputDelegateHandler = Utils::Signal<float, MotionType>;

    public:
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
            BindType   Type,
            float      Factor,
            MotionType Motion);

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
            m_OnWheelVertical,
            m_OnWheelHorizontal;
    };

    class InputMouseWheelDataEvent final
    {
    public:
        InputMouseWheelDataEvent(
            float                       Factor,
            InputMouseWheel::BindType   Type,
            InputMouseWheel::MotionType Motion);

        /// <summary>
        /// Dispatch the input type.
        /// </summary>
        void DispatchInput(
            class IInputMouseWheelTable* Table);

    private:
        float                       m_Factor;
        InputMouseWheel::BindType   m_InputType;
        InputMouseWheel::MotionType m_Motion;
    };
} // namespace Neon::Input