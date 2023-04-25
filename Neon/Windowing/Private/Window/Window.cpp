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

            MiniDumpWriteDumpFn WriteMiniDump = std::bit_cast<MiniDumpWriteDumpFn>(GetProcAddress(
                std::bit_cast<HMODULE>(DBGHelp.get()),
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
            else
            {
                return EXCEPTION_CONTINUE_SEARCH;
            }
        }
    } // namespace Impl

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
        m_WindowSize(Size)
    {
        Impl::InitializeClassName();

        Size2I FinalSize = Size;

        HDC screenDC = GetDC(nullptr);
        int Left     = (GetDeviceCaps(screenDC, HORZRES) - FinalSize.Width()) / 2;
        int Top      = (GetDeviceCaps(screenDC, VERTRES) - FinalSize.Height()) / 2;
        ReleaseDC(nullptr, screenDC);

        DWORD WinStyle = WS_VISIBLE;
        if (Style.None())
        {
            WinStyle |= WS_POPUP;
        }
        else
        {
            if (Style.Test(EWindowStyle::TitleBar))
                WinStyle |= WS_CAPTION | WS_MINIMIZEBOX;
            if (Style.Test(EWindowStyle::Resize))
                WinStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;
            if (Style.Test(EWindowStyle::Close))
                WinStyle |= WS_SYSMENU;
        }

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
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            FinalSize.Width(),
            FinalSize.Height(),
            nullptr,
            nullptr,
            GetModuleHandle(nullptr),
            this);

        SetWindowSubclass(m_Handle, &WindowApp::WndSubClassProc, NULL, std::bit_cast<DWORD_PTR>(this));
    }

    WindowApp::~WindowApp()
    {
        DestroyWindow(m_Handle);

        Impl::ShutdownClassName();
    }

    void* WindowApp::GetPlatformHandle() const
    {
        throw std::logic_error("Not implemented");
    }

    void WindowApp::Close()
    {
        throw std::logic_error("Not implemented");
    }

    Vector2DI WindowApp::GetPosition() const
    {
        throw std::logic_error("Not implemented");
    }

    void WindowApp::SetPosition(
        const Vector2DI& Position)
    {
        throw std::logic_error("Not implemented");
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
        throw std::logic_error("Not implemented");
    }

    void WindowApp::SetIcon(
        const StringU8& IconPath)
    {
        throw std::logic_error("Not implemented");
    }

    void WindowApp::SetIcon(
        const void*      IconData,
        const Vector2DI& Size)
    {
        throw std::logic_error("Not implemented");
    }

    void WindowApp::SetVisible(
        bool Show)
    {
        throw std::logic_error("Not implemented");
    }

    bool WindowApp::IsVisible() const
    {
        throw std::logic_error("Not implemented");
    }

    void WindowApp::Focus(bool Show)
    {
        throw std::logic_error("Not implemented");
    }

    bool WindowApp::hasFocus() const
    {
        throw std::logic_error("Not implemented");
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

} // namespace Neon::Windowing