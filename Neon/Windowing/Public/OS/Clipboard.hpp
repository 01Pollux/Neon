#pragma once

#include <Core/String.hpp>
#include <filesystem>
#include <span>

struct GLFWwindow;

namespace Neon::OS
{
    /// <summary>
    /// Get the current clipboard text
    /// </summary>
    [[nodiscard]] StringU8 GetClipboard(
        GLFWwindow* Window);

    /// <summary>
    /// Set the current clipboard text
    /// </summary>
    void SetClipboard(
        GLFWwindow*     Window,
        const StringU8& Clipboard);
} // namespace Neon::OS