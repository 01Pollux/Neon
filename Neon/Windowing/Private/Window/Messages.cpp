#include <WindowPCH.hpp>
#include <Private/Window/Window.hpp>

#include <CommCtrl.h>

#include <Log/Logger.hpp>

namespace Neon::Windowing
{
    LRESULT WindowApp::WndProc(
        HWND   Handle,
        UINT   Message,
        WPARAM wParam,
        LPARAM lParam)
    {
        auto Window = std::bit_cast<WindowApp*>(GetWindowLongPtr(Handle, GWLP_USERDATA));
        switch (Message)
        {
        [[likely]] default:
        {
            Window->ProcessMessage(Message, wParam, lParam);
            break;
        }

        [[unlikely]] case WM_DESTROY:
        {
            NEON_TRACE_TAG(
                "Window", "Destroying Window: {}",
                StringUtils::Transform<StringU8>(Window->GetTitle()));
            PostQuitMessage(0);
            return 0;
        }

        [[unlikely]] case WM_CREATE:
        {
            CREATESTRUCT* CreateInfo = std::bit_cast<CREATESTRUCT*>(lParam);
            SetWindowLongPtr(Handle, GWLP_USERDATA, std::bit_cast<LONG_PTR>(CreateInfo->lpCreateParams));

            WindowApp* Window = std::bit_cast<WindowApp*>(CreateInfo->lpCreateParams);
            Window->m_Handle  = Handle;
            break;
        }
        }

        return DefWindowProc(Handle, Message, wParam, lParam);
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
            UINT   Width  = LOWORD(lParam);
            UINT   Height = HIWORD(lParam);
            Size2I Size{ int(Width), int(Height) };
            if (wParam != SIZE_MINIMIZED &&
                !m_WindowFlags.Test(EWindowFlags::Resizing) &&
                m_WindowSize != Size)
            {
                m_WindowSize = Size;
                QueueEvent(Events::SizeChanged{
                    .NewSize = Size });
            }
            break;
        }
        case WM_ENTERSIZEMOVE:
        {
            m_WindowFlags.Set(EWindowFlags::Resizing);
            break;
        }
        case WM_EXITSIZEMOVE:
        {
            Size2I Size = GetSize();
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