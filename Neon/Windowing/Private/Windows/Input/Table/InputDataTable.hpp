#pragma once

#include <Input/Table/InputDataTable.hpp>
#include <Private/Windows/API/WindowHeaders.hpp>

namespace Neon::Input
{
    class InputDataTableImpl : public IInputDataTable
    {
    public:
        /// <summary>
        /// Pushes a message to the input data table.
        /// </summary>
        LRESULT PushMessage(
            HWND   WindowHandle,
            UINT   Message,
            WPARAM wParam,
            LPARAM lParam,
            bool&  Handled);
    };
} // namespace Neon::Input