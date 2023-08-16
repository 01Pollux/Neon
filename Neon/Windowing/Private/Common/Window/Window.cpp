#include <WindowPCH.hpp>
#include <Window/Window.hpp>
#include <Input/Table.hpp>

#ifdef NEON_PLATFORM_WINDOWS
#include <ImGui/backends/imgui_impl_win32.h>
#endif

#include <glfw/glfw3.h>
#include <Log/Logger.hpp>

namespace Neon::Windowing
{
    static std::atomic_uint32_t s_GlfwInitCount = 0;

    static void GlfwErrorCallback(
        int         Code,
        const char* Description)
    {
    }

    //

    WindowApp::WindowApp(
        const WindowInitDesc& Desc) :
        m_Title(Desc.Title)
    {
        if (s_GlfwInitCount.load(std::memory_order_acquire) == 0)
        {
            NEON_ASSERT(glfwInit() == GLFW_TRUE);

            glfwSetErrorCallback(GlfwErrorCallback);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

#ifdef NEON_PLATFORM_WINDOWS
            glfwWindowHint(GLFW_TITLEBAR, false);
#else
            glfwWindowHint(GLFW_DECORATED, false);
#endif

            s_GlfwInitCount.store(1, std::memory_order_release);
        }

        GLFWmonitor*       Monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* Mode    = glfwGetVideoMode(Monitor);

        if (Desc.FullScreen)
        {
            glfwWindowHint(GLFW_DECORATED, false);
            glfwWindowHint(GLFW_RED_BITS, Mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, Mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, Mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, Mode->refreshRate);

            m_Handle = glfwCreateWindow(Mode->width, Mode->height, Desc.Title.c_str(), Monitor, nullptr);
        }
        else
        {
            m_Handle = glfwCreateWindow(Desc.Size.Width(), Desc.Size.Height(), Desc.Title.c_str(), nullptr, nullptr);

            if (Desc.StartInMiddle)
            {
                int X = (Mode->width - Desc.Size.Width()) / 2;
                int Y = (Mode->height - Desc.Size.Height()) / 2;

                glfwSetWindowPos(m_Handle, X, Y);
            }
        }

        glfwSetWindowUserPointer(m_Handle, this);

        //

        glfwSetWindowSizeCallback(
            m_Handle,
            [](GLFWwindow* Window, int Width, int Height)
            {
                auto App = static_cast<WindowApp*>(glfwGetWindowUserPointer(Window));

                App->m_WindowSize = Size2I{ Width, Height };
                App->OnWindowSizeChanged().Broadcast(App->m_WindowSize);
            });

        glfwSetWindowCloseCallback(
            m_Handle,
            [](GLFWwindow* Window)
            {
                auto App = static_cast<WindowApp*>(glfwGetWindowUserPointer(Window));
                App->OnWindowClosed().Broadcast();
            });

        SetupInputs();

        glfwSetTitlebarHitTestCallback(
            m_Handle,
            [](GLFWwindow* Window, int X, int Y, int* Hit)
            {
                auto App    = static_cast<WindowApp*>(glfwGetWindowUserPointer(Window));
                bool WasHit = false;
                App->OnWindowTitleHitTest().Broadcast({ X, Y }, WasHit);
                *Hit = WasHit;
            });

        glfwSetWindowIconifyCallback(
            m_Handle,
            [](GLFWwindow* Window, int Iconified)
            {
                auto App    = static_cast<WindowApp*>(glfwGetWindowUserPointer(Window));
                bool WasHit = false;
                App->OnWindowMinized().Broadcast(Iconified);
            });

        int Width, Height;
        glfwGetWindowSize(m_Handle, &Width, &Height);
        m_WindowSize = Size2I{ Width, Height };
    }

    WindowApp::~WindowApp()
    {
        glfwDestroyWindow(m_Handle);

        if (s_GlfwInitCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            glfwTerminate();
        }
    }

    GLFWwindow* WindowApp::GetHandle() const
    {
        return m_Handle;
    }

    void WindowApp::Close()
    {
        glfwSetWindowShouldClose(m_Handle, GLFW_TRUE);
    }

    bool WindowApp::IsRunning() const
    {
        return glfwWindowShouldClose(m_Handle) == GLFW_FALSE;
    }

    const StringU8& WindowApp::GetTitle() const
    {
        return m_Title;
    }

    void WindowApp::SetTitle(
        StringU8 Title)
    {
        m_Title = std::move(Title);
        glfwSetWindowTitle(m_Handle, m_Title.c_str());
    }

    Vector2I WindowApp::GetPosition() const
    {
        int X, Y;
        glfwGetWindowPos(m_Handle, &X, &Y);
        return { X, Y };
    }

    void WindowApp::SetPosition(
        const Vector2I& Position)
    {
        glfwSetWindowPos(m_Handle, Position.x, Position.y);
    }

    Size2I WindowApp::GetSize() const
    {
        return m_WindowSize;
    }

    bool WindowApp::IsMinimized() const
    {
        return glfwGetWindowAttrib(m_Handle, GLFW_ICONIFIED) == GLFW_TRUE;
    }

    bool WindowApp::IsMaximized() const
    {
        return glfwGetWindowAttrib(m_Handle, GLFW_MAXIMIZED) == GLFW_TRUE;
    }

    bool WindowApp::IsVisible() const
    {
        return glfwGetWindowAttrib(m_Handle, GLFW_VISIBLE) == GLFW_TRUE;
    }

    void WindowApp::SetSize(
        const Size2I& Size)
    {
        glfwSetWindowSize(m_Handle, Size.Width(), Size.Height());
        m_WindowSize = Size;
    }

    void WindowApp::SetIcon(
        void*           IconData,
        const Vector2I& Size)
    {
        // Set icon for window
        GLFWimage Image{
            .width  = Size.x,
            .height = Size.y,
            .pixels = const_cast<uint8_t*>(static_cast<const uint8_t*>(IconData))
        };
        glfwSetWindowIcon(m_Handle, 1, &Image);
    }

    void WindowApp::SetVisible(
        bool Show)
    {
        if (Show)
        {
            glfwShowWindow(m_Handle);
        }
        else
        {
            glfwHideWindow(m_Handle);
        }
    }

    void WindowApp::RequestFocus()
    {
        glfwRequestWindowAttention(m_Handle);
    }

    bool WindowApp::HasFocus() const
    {
        return glfwGetWindowAttrib(m_Handle, GLFW_FOCUSED) == GLFW_TRUE;
    }

    //

    void WindowApp::RegisterInputTable(
        const Ptr<Input::InputDataTable>& InputTable)
    {
        InputTable->ClearState();
        m_InputTables.insert(InputTable);
    }

    void WindowApp::UnregisterInputTable(
        const Ptr<Input::InputDataTable>& InputTable)
    {
        InputTable->ClearState();
        m_InputTables.erase(InputTable);
    }

    void WindowApp::ProcessInputs()
    {
        for (const auto& InputTable : m_InputTables)
        {
            InputTable->ProcessInputs();
        }
    }

    void WindowApp::SetupInputs()
    {
        glfwSetKeyCallback(
            m_Handle,
            [](GLFWwindow* Window, int Key, int ScanCode, int Action, int Mods)
            {
                auto App = static_cast<WindowApp*>(glfwGetWindowUserPointer(Window));
                for (auto& InputTable : App->m_InputTables)
                {
                    InputTable->PushKey(Key, ScanCode, Action, Mods);
                }
            });

        glfwSetMouseButtonCallback(
            m_Handle,
            [](GLFWwindow* Window, int Button, int Action, int Mods)
            {
                auto App = static_cast<WindowApp*>(glfwGetWindowUserPointer(Window));
                for (auto& InputTable : App->m_InputTables)
                {
                    InputTable->PushMouseInput(Button, Action, Mods);
                }
            });

        glfwSetCursorPosCallback(
            m_Handle,
            [](GLFWwindow* Window, double X, double Y)
            {
                auto App = static_cast<WindowApp*>(glfwGetWindowUserPointer(Window));
                for (auto& InputTable : App->m_InputTables)
                {
                    InputTable->PushMouseMove(X, Y);
                }
            });
    }
} // namespace Neon::Windowing