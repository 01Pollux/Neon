#pragma once

#include <Private/Windows/Input/Table/InputDataTable.hpp>

namespace Neon::Input
{
    class InputMouseTableImpl : public IInputMouseTable
    {
    public:
        /// <summary>
        /// Pushes a message to the input data table.
        /// </summary>
        bool PushMessage(
            InputEventQueue& EventQueue,
            HWND             WindowHandle,
            UINT             Message,
            WPARAM           wParam,
            LPARAM           lParam);

    private:
        /// <summary>
        /// Reads the mouse input.
        /// </summary>
        bool ReadMouseInput(
            HWND                   WindowHandle,
            UINT                   Message,
            WPARAM                 wParam,
            LPARAM                 lParam,
            Vector2&               MousePosition,
            EMouseInput&           InputType,
            IInputMouse::BindType& Type);

    private:
        /// <summary>
        /// Used to precache the raw input pool from GetRawInputData function.
        /// </summary>
        std::vector<uint8_t> m_RawInputPool;
        POINT                m_OldMousePosition{};
    };
} // namespace Neon::Input
