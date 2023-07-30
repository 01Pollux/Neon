#include <WindowPCH.hpp>
#include <Private/Windows/Window/Window.hpp>
#include <Private/Windows/Input/Table.hpp>

#include <CommCtrl.h>

#include <Log/Logger.hpp>

namespace Neon::Windowing
{
    void WindowApp::PostTaskMessage() const
    {
        if (!PostMessage(m_Handle, WM_USER_TASK_PENDING, 0, 0))
        {
            NEON_ASSERT(false, "Failed to post task message: {:X}", GetLastError());
        }
    }

    void WindowApp::WindowThread()
    {
        m_WindowCreatedLatch.wait();
        m_TaskQueue.PopExecute();

        MSG Message;
        while (GetMessage(&Message, nullptr, 0, 0))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }

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
            if (auto Ret = Window->ProcessMessage(Message, wParam, lParam))
            {
                return *Ret;
            }
            break;
        }

        case WM_USER_TASK_PENDING:
        {
            Window->m_TaskQueue.PopExecute();
            return 0;
        }

        case WM_DESTROY:
        {
            NEON_TRACE_TAG(
                "Window", "Destroying Window: {}",
                StringUtils::Transform<StringU8>(Window->GetWindowTitle()));
            Window->m_Handle = nullptr;
            PostQuitMessage(0);
            return 0;
        }

        case WM_CREATE:
        {
            CREATESTRUCT* CreateInfo = std::bit_cast<CREATESTRUCT*>(lParam);
            SetWindowLongPtr(Handle, GWLP_USERDATA, std::bit_cast<LONG_PTR>(CreateInfo->lpCreateParams));
            break;
        }

        case WM_CLOSE:
        {
            Window->m_IsRunning = false;
            return 0;
        }
        }

        return DefWindowProc(Handle, Message, wParam, lParam);
    }

    //

    void WindowApp::QueueEvent(
        Event Msg)
    {
        std::scoped_lock Lock(m_PendingEventsMutex);
        m_PendingEvents.emplace(std::move(Msg));
    }

    std::optional<LRESULT> WindowApp::ProcessMessage(
        UINT   Message,
        WPARAM wParam,
        LPARAM lParam)
    {
        std::optional<LRESULT> FinalRet;

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

            switch (wParam)
            {
            case SIZE_MINIMIZED:
            {
                m_WindowFlags.Set(EWindowFlags::Minimized);
                m_WindowFlags.Set(EWindowFlags::Maximized, false);
                break;
            }
            case SIZE_MAXIMIZED:
            {
                m_WindowFlags.Set(EWindowFlags::Maximized);
                m_WindowFlags.Set(EWindowFlags::Minimized, false);
                break;
            }
            case SIZE_RESTORED:
            {
                m_WindowFlags.Set(EWindowFlags::Minimized, false);
                m_WindowFlags.Set(EWindowFlags::Maximized, false);
                break;
            }
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
            Size2I Size = GetWindowSize();
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

        for (auto& InputTable : m_InputTables)
        {
            bool ProcessInput = false;
            auto Ret          = validate_cast<Input::InputDataTableImpl*>(InputTable.get())->PushMessage(this->m_Handle, Message, wParam, lParam, ProcessInput);
            if (ProcessInput)
            {
                FinalRet = Ret;
            }
        }

        return FinalRet;
    }

    Size2I WindowApp::GetWindowSize() const
    {
        RECT Rect;
        GetClientRect(m_Handle, &Rect);
        return { Rect.right - Rect.left, Rect.bottom - Rect.top };
    }

    String WindowApp::GetWindowTitle() const
    {
        int    TitleLength = GetWindowTextLengthW(m_Handle) + 1;
        String Title(TitleLength, STR('\0'));
        GetWindowTextW(m_Handle, Title.data(), TitleLength);
        return Title;
    }
} // namespace Neon::Windowing