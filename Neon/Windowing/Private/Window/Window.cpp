#include <WindowPCH.hpp>
#include <Private/Window/Window.hpp>

#include <consoleapi.h>
#include <CommCtrl.h>
#include <minidumpapiset.h>

#include <Log/Logger.hpp>

namespace Neon::Windowing
{
    namespace Impl
    {
        static constexpr auto* s_ClassName        = STR("__NEON01");
        static LPVOID          s_ExceptionHandler = nullptr;

        static inline std::mutex s_ClassNameInitMutex;
        static inline int        s_ClassNameInits = 0;

#if !NEON_DIST
        BOOL ConsoleCloseRoutine(
            DWORD CtrlType)
        {
            return CtrlType == CTRL_CLOSE_EVENT;
        }
#endif

        void InitializeClassName()
        {
            std::lock_guard ClassInit(s_ClassNameInitMutex);
            if (!s_ClassNameInits++)
            {
                NEON_TRACE_TAG(
                    "Window", "Registering Classname: {}",
                    StringUtils::Transform<StringU8>(s_ClassName));

#if !NEON_DIST
                NEON_ASSERT(SetConsoleCtrlHandler(ConsoleCloseRoutine, TRUE) != 0);
#endif

                WNDCLASSEX WndClass{};
                WndClass.cbSize        = sizeof(WndClass);
                WndClass.style         = CS_OWNDC | CS_DBLCLKS;
                WndClass.lpfnWndProc   = &WindowApp::WndProc;
                WndClass.hInstance     = GetModuleHandle(nullptr);
                WndClass.lpszClassName = s_ClassName;

                NEON_VALIDATE(RegisterClassEx(&WndClass) != 0);
                NEON_VALIDATE(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));
            }
        }

        void ShutdownClassName()
        {
            std::lock_guard ClassInit(s_ClassNameInitMutex);
            if (!--s_ClassNameInits)
            {
                NEON_TRACE_TAG(
                    "Window", "Unregistering Classname: {}",
                    StringUtils::Transform<StringU8>(s_ClassName));

#if !NEON_DIST
                NEON_ASSERT(SetConsoleCtrlHandler(ConsoleCloseRoutine, FALSE) != 0);
#endif

                CoUninitialize();
                NEON_VALIDATE(UnregisterClass(s_ClassName, GetModuleHandle(nullptr)) != 0);

                RemoveVectoredExceptionHandler(s_ExceptionHandler);
            }
        }
    } // namespace Impl

    //

    IWindowApp* IWindowApp::Create(
        const String&       Title,
        const Size2I&       Size,
        const MWindowStyle& Style,
        bool                StartInMiddle)
    {
        return NEON_NEW WindowApp(Title, Size, Style, StartInMiddle);
    }

    WindowApp::WindowApp(
        const String&       Title,
        const Size2I&       Size,
        const MWindowStyle& Style,
        bool                StartInMiddle) :
        m_WindowSize(Size),
        m_WindowStyle(Style)
    {
        NEON_TRACE_TAG(
            "Window", "Creating Window '{}' -- Size: {}x{}",
            StringUtils::Transform<StringU8>(Title),
            Size.Width(),
            Size.Height());

        Impl::InitializeClassName();

        Size2I FinalSize = Size;

        HDC WindowDC = GetDC(nullptr);
        int Left, Top;
        if (!StartInMiddle)
        {
            Left = Top = CW_USEDEFAULT;
        }
        else
        {
            Left = (GetDeviceCaps(WindowDC, HORZRES) - FinalSize.Width()) / 2;
            Top  = (GetDeviceCaps(WindowDC, VERTRES) - FinalSize.Height()) / 2;
        }
        m_BitsPerPixel = GetDeviceCaps(WindowDC, BITSPIXEL);
        ReleaseDC(nullptr, WindowDC);

        DWORD WinStyle = GetWindowStyle(Style);

        if (!Style.Test(EWindowStyle::Fullscreen))
        {
            RECT Rect = { 0, 0, FinalSize.Width(), FinalSize.Height() };
            AdjustWindowRect(&Rect, WinStyle, false);
            FinalSize.Width(Rect.right - Rect.left);
            FinalSize.Height(Rect.bottom - Rect.top);
        }

        CreateWindowExW(
            0,
            Impl::s_ClassName,
            Title.c_str(),
            WinStyle,
            Left,
            Top,
            FinalSize.Width(),
            FinalSize.Height(),
            nullptr,
            nullptr,
            GetModuleHandle(nullptr),
            this);

        if (Style.Test(EWindowStyle::Fullscreen))
        {
            SwitchToFullscreen();
        }
    }

    WindowApp::~WindowApp()
    {
        DestroyWindow(m_Handle);
        Impl::ShutdownClassName();
    }

    void* WindowApp::GetPlatformHandle() const
    {
        return m_Handle;
    }

    void WindowApp::Close()
    {
        PostMessage(m_Handle, WM_CLOSE, 0, 0);
    }

    String WindowApp::GetTitle() const
    {
        int    TitleLength = GetWindowTextLengthW(m_Handle) + 1;
        String Title(TitleLength, '\0');
        GetWindowTextW(m_Handle, Title.data(), TitleLength);
        return Title;
    }

    void WindowApp::SetTitle(
        const String& Title)
    {
        SetWindowTextW(m_Handle, Title.c_str());
    }

    MWindowStyle WindowApp::GetStyle() const
    {
        return m_WindowStyle;
    }

    void WindowApp::SetStyle(
        const MWindowStyle& Style)
    {
        m_WindowStyle = Style;

        DWORD WinStyle = GetWindowStyle(m_WindowStyle);
        SetWindowLong(m_Handle, GWL_STYLE, WinStyle);

        if (m_WindowStyle.Test(EWindowStyle::Fullscreen))
        {
            SwitchToFullscreen();
        }
        else
        {
            RECT Rect;
            GetClientRect(m_Handle, &Rect);

            SetWindowPos(
                m_Handle,
                HWND_NOTOPMOST,
                Rect.left,
                Rect.top,
                Rect.right - Rect.left,
                Rect.bottom - Rect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ShowWindow(m_Handle, SW_NORMAL);
        }
    }

    Size2I WindowApp::GetScreenCaps() const
    {
        HDC    WindowDC = GetDC(m_Handle);
        Size2I Size{
            GetDeviceCaps(WindowDC, DESKTOPHORZRES),
            GetDeviceCaps(WindowDC, DESKTOPVERTRES)
        };
        ReleaseDC(m_Handle, WindowDC);
        return Size;
    }

    Vector2I WindowApp::GetPosition() const
    {
        RECT Rect;
        GetClientRect(m_Handle, &Rect);
        return { Rect.left, Rect.top };
    }

    void WindowApp::SetPosition(
        const Vector2I& Position)
    {
        SetWindowPos(m_Handle, nullptr, Position.x, Position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    Size2I WindowApp::GetSize() const
    {
        RECT Rect;
        GetClientRect(m_Handle, &Rect);
        return { Rect.right - Rect.left, Rect.bottom - Rect.top };
    }

    void WindowApp::SetSize(
        const Size2I& Size)
    {
        RECT Rect = { 0, 0, Size.Width(), Size.Height() };
        AdjustWindowRect(&Rect, static_cast<DWORD>(GetWindowLongPtr(m_Handle, GWL_STYLE)), false);

        int Width  = Rect.right - Rect.left;
        int Height = Rect.bottom - Rect.top;

        SetWindowPos(m_Handle, nullptr, 0, 0, Width, Height, SWP_NOMOVE | SWP_NOZORDER);
    }

    bool WindowApp::IsMinimized() const
    {
        return m_WindowFlags.Test(EWindowFlags::Minimized);
    }

    bool WindowApp::IsMaximized() const
    {
        return m_WindowFlags.Test(EWindowFlags::Maximized);
    }

    bool WindowApp::IsVisible() const
    {
        return !IsMinimized() && IsWindowVisible(m_Handle);
    }

    void WindowApp::SetIcon(
        const void*     IconData,
        const Vector2I& Size)
    {
        if (m_Icon)
            DestroyIcon(m_Icon);

        if (!IconData)
        {
            m_Icon = nullptr;
            return;
        }

        // Create the icon from the pixel array
        m_Icon = CreateIcon(
            GetModuleHandleW(nullptr),
            Size.x,
            Size.y,
            1,
            32,
            nullptr,
            std::bit_cast<const uint8_t*>(IconData));

        if (m_Icon)
        {
            SendMessageW(m_Handle, WM_SETICON, ICON_BIG, std::bit_cast<LPARAM>(m_Icon));
            SendMessageW(m_Handle, WM_SETICON, ICON_SMALL, std::bit_cast<LPARAM>(m_Icon));
        }
        else
        {
            NEON_WARNING_TAG("Window", "Failed to set the window's icon");
        }
    }

    void WindowApp::SetVisible(
        bool Show)
    {
        ShowWindow(m_Handle, Show ? SW_SHOW : SW_HIDE);
    }

    void WindowApp::RequestFocus()
    {
        if (HasFocus())
        {
            SetForegroundWindow(m_Handle);
        }
        else
        {
            FLASHWINFO Info{
                .cbSize    = sizeof(Info),
                .hwnd      = m_Handle,
                .dwFlags   = FLASHW_TRAY,
                .uCount    = 3,
                .dwTimeout = 0
            };
            FlashWindowEx(&Info);
        }
    }

    bool WindowApp::HasFocus() const
    {
        return m_Handle == GetForegroundWindow();
    }

    bool WindowApp::PeekEvent(
        Event* Msg,
        bool   Erase,
        bool   Block)
    {
        if (m_PendingEvents.empty())
        {
            ProcessMessages();
            if (Block)
            {
                while (m_PendingEvents.empty())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    ProcessMessages();
                }
            }
            else if (m_PendingEvents.empty())
            {
                return false;
            }
        }

        *Msg = m_PendingEvents.back();
        if (Erase)
        {
            m_PendingEvents.pop();
        }

        return true;
    }

    //

    DWORD WindowApp::GetWindowStyle(
        const MWindowStyle& Flags)
    {
        DWORD WinStyle = WS_VISIBLE;
        if (Flags.Test(EWindowStyle::Fullscreen) && !Flags.Test(EWindowStyle::TitleBar))
        {
            WinStyle |= WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        }
        else
        {
            if (Flags.Test(EWindowStyle::TitleBar))
                WinStyle |= WS_CAPTION | WS_MINIMIZEBOX;
            if (Flags.Test(EWindowStyle::Resize))
                WinStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;
            if (Flags.Test(EWindowStyle::Close))
                WinStyle |= WS_SYSMENU;

            if (WinStyle == WS_VISIBLE)
                WinStyle |= WS_POPUP;
        }

        if (Flags.TestNone(BitMask_Or(EWindowStyle::Fullscreen, EWindowStyle::Windowed)))
        {
            NEON_WARNING_TAG("Window", "Window must be either fullscreen, fullscreen borderless or windowed");
        }
        return WinStyle;
    }

    void WindowApp::SwitchToFullscreen()
    {
        if (m_WindowStyle.Test(EWindowStyle::Windowed))
        {
            ShowWindow(m_Handle, SW_MAXIMIZE);
            return;
        }

        constexpr DWORD Flags = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        SetWindowLongPtr(m_Handle, GWL_STYLE, LONG_PTR(Flags));
        SetWindowLongPtr(m_Handle, GWL_EXSTYLE, WS_EX_APPWINDOW);

        SetWindowPos(m_Handle, HWND_TOP, 0, 0, m_WindowSize.Width(), m_WindowSize.Height(), SWP_FRAMECHANGED);
        ShowWindow(m_Handle, SW_SHOW);
    }
} // namespace Neon::Windowing