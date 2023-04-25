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
        if (Message == WM_DESTROY) [[unlikely]]
        {
            PostQuitMessage(0);
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

    //

    void WindowApp::ProcessMessages()
    {
        MSG Msg;
        while (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (Msg.message == WM_QUIT)
            {
                QueueEvent(Events::Close{ .ExitCode = int(Msg.wParam) });
            }
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

    void WindowApp::QueueEvent(
        Event Msg)
    {
        m_PendingEvents.emplace(std::move(Msg));
    }

    void WindowApp::ProcessMessage(
        UINT   Message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (Message)
        {
        case WM_SIZE:
        {
            Size2I Size = GetSize(); // TODO: Use wParam

            if (wParam != SIZE_MINIMIZED &&
                !m_WindowFlags.Test(EWindowFlags::Resizing) &&
                m_WindowSize != Size)
            {
                m_WindowSize = Size;
                QueueEvent(Events::SizeChanged{
                    .NewSize = Size });
            }
        }
        case WM_ENTERSIZEMOVE:
        {
            m_WindowFlags.Set(EWindowFlags::Resizing);
            break;
        }
        case WM_EXITSIZEMOVE:
        {
            Size2I Size = GetSize(); // TODO: Use wParam
            m_WindowFlags.Set(EWindowFlags::Resizing, false);

            if (m_WindowSize != Size)
            {
                m_WindowSize = Size;
                QueueEvent(Events::SizeChanged{
                    .NewSize = Size });
            }
            break;
        }
        }
    }
} // namespace Neon::Windowing