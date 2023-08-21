#include <WindowPCH.hpp>
#include <OS/Clipboard.hpp>

#include <glfw/glfw3.h>

namespace Neon::OS
{
    StringU8 GetClipboard(
        GLFWwindow* Window)
    {
        return glfwGetClipboardString(Window);
    }

    void SetClipboard(
        GLFWwindow*     Window,
        const StringU8& Clipboard)
    {
        glfwSetClipboardString(Window, Clipboard.c_str());
    }
} // namespace Neon::OS
