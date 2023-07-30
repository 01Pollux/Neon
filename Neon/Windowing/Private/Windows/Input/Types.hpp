#pragma once

#include <Input/Types.hpp>

namespace Neon::Input
{
    /// <summary>
    /// Convert Keyboard input to platform specific input.
    /// </summary>
    unsigned int KeyboardInputToPlatform(
        EKeyboardInput KeyCode);

    /// <summary>
    /// Convert Mouse input to platform specific input.
    /// </summary>
    EKeyboardInput KeyboardInputFromPlatform(
        unsigned long long KeyCode,
        unsigned long long lParam);

    /// <summary>
    /// Convert Mouse input to platform specific input.
    /// </summary>
    EMouseInput MouseInputFromPlatform(
        unsigned long      MouseCode,
        unsigned long long wParam);
} // namespace Neon::Input
