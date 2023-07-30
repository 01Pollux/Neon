#pragma once

#include <Input/Data/InputData.hpp>
#include <Utils/Signal.hpp>

namespace Neon::Input
{
    class IInputMouseWheel
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
            Ref<IInputMouseWheel>        Mouse,
            float                        Factor,
            IInputMouseWheel::BindType   Type,
            IInputMouseWheel::MotionType Motion);

        void DispatchInput();

    private:
        Ref<IInputMouseWheel>        m_InputAxis;
        float                        m_Factor;
        IInputMouseWheel::BindType   m_InputType;
        IInputMouseWheel::MotionType m_Motion;
    };
} // namespace Neon::Input