#pragma once

#include <Private/Windows/Input/Table/InputDataTable.hpp>

namespace Neon::Input
{
    class InputMouseWheelTableImpl : public IInputMouseWheelTable
    {
    public:
        /// <summary>
        /// Pushes a message to the input data table.
        /// </summary>
        bool PushMessage(
            InputEventQueue& EventQueue,
            UINT             Message,
            WPARAM           wParam,
            LPARAM           lParam);
    };
} // namespace Neon::Input
