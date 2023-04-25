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

        static LONG NTAPI OnRaiseException(
            _In_ PEXCEPTION_POINTERS ExceptionInfo);

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
                s_ExceptionHandler = AddVectoredExceptionHandler(TRUE, &OnRaiseException);

#if !NEON_DIST
                int ReportFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
                _CrtSetDbgFlag(ReportFlags | _CRTDBG_LEAK_CHECK_DF);
                NEON_ASSERT(SetConsoleCtrlHandler(ConsoleCloseRoutine, TRUE) != 0);
#endif

                WNDCLASSEX WndClass{};
                WndClass.cbSize        = sizeof(WndClass);
                WndClass.style         = CS_OWNDC | CS_DBLCLKS;
                WndClass.lpfnWndProc   = &WindowApp::WndProc;
                WndClass.hInstance     = GetModuleHandle(nullptr);
                WndClass.lpszClassName = s_ClassName;

                NEON_ASSERT(RegisterClassEx(&WndClass) != 0);
                NEON_ASSERT(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));
            }
        }

        void ShutdownClassName()
        {
            std::lock_guard ClassInit(s_ClassNameInitMutex);
            if (!--s_ClassNameInits)
            {
#if !NEON_DIST
                NEON_ASSERT(SetConsoleCtrlHandler(ConsoleCloseRoutine, FALSE) != 0);
#endif

                CoUninitialize();
                NEON_ASSERT(UnregisterClass(s_ClassName, GetModuleHandle(nullptr)) != 0);

                RemoveVectoredExceptionHandler(s_ExceptionHandler);
            }
        }

        LONG NTAPI OnRaiseException(_In_ PEXCEPTION_POINTERS ExceptionInfo)
        {
            using LibraryPtr = std::unique_ptr<
                void,
                decltype([](void* Handle)
                         { FreeLibrary(std::bit_cast<HMODULE>(Handle)); })>;

            LibraryPtr DBGHelp{ static_cast<void*>(LoadLibraryW(STR("DBGHelp.dll"))) };
            if (!DBGHelp)
                return EXCEPTION_CONTINUE_SEARCH;

            switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
            {
            case EXCEPTION_ACCESS_VIOLATION:
            case EXCEPTION_INVALID_HANDLE:
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            case EXCEPTION_DATATYPE_MISALIGNMENT:
            case EXCEPTION_ILLEGAL_INSTRUCTION:
            case EXCEPTION_INT_DIVIDE_BY_ZERO:
            case EXCEPTION_STACK_OVERFLOW:
            case EXCEPTION_STACK_INVALID:
            case EXCEPTION_WRITE_FAULT:
            case EXCEPTION_READ_FAULT:
            case STATUS_STACK_BUFFER_OVERRUN:
            case STATUS_HEAP_CORRUPTION:
                break;
            case EXCEPTION_BREAKPOINT:
                if (!(ExceptionInfo->ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE))
                    return EXCEPTION_CONTINUE_SEARCH;
                break;
            default:
                return EXCEPTION_CONTINUE_SEARCH;
            }

            using MiniDumpWriteDumpFn =
                BOOL(WINAPI*)(
                    _In_ HANDLE                                hProcess,
                    _In_ DWORD                                 ProcessId,
                    _In_ HANDLE                                hFile,
                    _In_ MINIDUMP_TYPE                         DumpType,
                    _In_opt_ PMINIDUMP_EXCEPTION_INFORMATION   ExceptionParam,
                    _In_opt_ PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                    _In_opt_ PMINIDUMP_CALLBACK_INFORMATION    CallbackParam);

            HANDLE hFile = CreateFileW(
                std::format(L"./Logs/Fatal_{0:%g}_{0:%h}_{0:%d}_{0:%H}_{0:%OM}_{0:%OS}.dmp", std::chrono::system_clock::now()).c_str(),
                GENERIC_WRITE,
                NULL,
                nullptr,
                CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);

            if (hFile == INVALID_HANDLE_VALUE)
                return EXCEPTION_CONTINUE_SEARCH;

            auto WriteMiniDump = std::bit_cast<MiniDumpWriteDumpFn>(GetProcAddress(
                HMODULE(DBGHelp.get()),
                "MiniDumpWriteDump"));

            MINIDUMP_EXCEPTION_INFORMATION MiniDumpInfo{
                .ThreadId          = GetCurrentThreadId(),
                .ExceptionPointers = ExceptionInfo,
                .ClientPointers    = FALSE
            };

            BOOL Res = WriteMiniDump(
                GetCurrentProcess(),
                GetCurrentProcessId(),
                hFile,
                static_cast<MINIDUMP_TYPE>(MiniDumpWithUnloadedModules | MiniDumpWithFullMemoryInfo | MiniDumpWithCodeSegs),
                &MiniDumpInfo,
                nullptr,
                nullptr);

            CloseHandle(hFile);

            if (Res)
            {
                ExceptionInfo->ExceptionRecord->ExceptionCode = EXCEPTION_BREAKPOINT;
                return EXCEPTION_EXECUTE_HANDLER;
            }
            return EXCEPTION_CONTINUE_SEARCH;
        }
    } // namespace Impl

    void SetFullscreenWithBorder(HWND Hwnd)
    {
        SetWindowLong(Hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
        RECT Rect;
        GetWindowRect(Hwnd, &Rect);
        SetWindowPos(
            Hwnd,
            HWND_TOPMOST,
            0,
            0,
            GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN),
            SWP_SHOWWINDOW);
    }

    //

    IWindowApp* IWindowApp::Create(
        const String&       Title,
        const Size2I&       Size,
        const MWindowStyle& Style)
    {
        return NEON_NEW WindowApp(Title, Size, Style);
    }

    WindowApp::WindowApp(
        const String&       Title,
        const Size2I&       Size,
        const MWindowStyle& Style) :
        m_WindowSize(Size),
        m_WindowStyle(Style)
    {
        Impl::InitializeClassName();

        Size2I FinalSize = Size;

        HDC WindowDC   = GetDC(nullptr);
        int Left       = (GetDeviceCaps(WindowDC, HORZRES) - FinalSize.Width()) / 2;
        int Top        = (GetDeviceCaps(WindowDC, VERTRES) - FinalSize.Height()) / 2;
        m_BitsPerPixel = GetDeviceCaps(WindowDC, BITSPIXEL);
        ReleaseDC(nullptr, WindowDC);

        DWORD WinStyle = GetWindowStyle(Style);

        if (!Style.Test(EWindowStyle::Fullscreen))
        {
            RECT Rect = { 0, 0, FinalSize.Width(), FinalSize.Height() };
            AdjustWindowRect(&Rect, WinStyle, false);
            FinalSize.Width()  = Rect.right - Rect.left;
            FinalSize.Height() = Rect.bottom - Rect.top;
        }

        m_Handle = CreateWindowExW(
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

        SetSize(Size);

        if (Style.Test(EWindowStyle::Fullscreen))
        {
            SwitchToFullscreen();
        }

        SetWindowSubclass(m_Handle, &WindowApp::WndSubClassProc, NULL, std::bit_cast<DWORD_PTR>(this));
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

    Vector2DI WindowApp::GetPosition() const
    {
        RECT Rect;
        GetClientRect(m_Handle, &Rect);
        return { Rect.left, Rect.top };
    }

    void WindowApp::SetPosition(
        const Vector2DI& Position)
    {
        SetWindowPos(m_Handle, nullptr, Position.x(), Position.y(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
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

    void WindowApp::SetIcon(
        const void*      IconData,
        const Vector2DI& Size)
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
            Size.x(),
            Size.y(),
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
            NEON_WARNING("Failed to set the window's icon");
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

        if (!Flags.Test(BitMask_Or(EWindowStyle::Fullscreen, EWindowStyle::Windowed)))
        {
            NEON_WARNING("Window must be either fullscreen, fullscreen borderless or windowed");
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