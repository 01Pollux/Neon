#include <WindowPCH.hpp>
#include <Private/Window/Window.hpp>

#include <CommCtrl.h>

namespace Neon::Windowing
{
    LRESULT WindowApp::WndProc(
        HWND   Handle,
        UINT   Message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (Message)
        {
        [[unlikely]] case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        }
        return DefWindowProc(Handle, Message, wParam, lParam);
    }

    LRESULT WindowApp::WndSubClassProc(
        HWND      Handle,
        UINT      Message,
        WPARAM    wParam,
        LPARAM    lParam,
        UINT_PTR  IdSubclass,
        DWORD_PTR RefData)
    {
        auto Window = std::bit_cast<WindowApp*>(RefData);
        switch (Message)
        {
        [[likely]] default:
        {
            Window->ProcessMessage(Message, wParam, lParam);
            break;
        }

        case WM_NCDESTROY:
        {
            RemoveWindowSubclass(Handle, &WindowApp::WndSubClassProc, IdSubclass);
            break;
        }
        }

        return DefSubclassProc(Handle, Message, wParam, lParam);
    }

    void WindowApp::ProcessMessage(
        UINT   Message,
        WPARAM wParam,
        LPARAM lParam)
    {
    }
} // namespace Neon::Windowing