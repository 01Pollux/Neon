#pragma once

#include <Window/WindowConfig.hpp>
#include <Utils/Signal.hpp>
#include <unordered_set>

namespace Neon::Input
{
    class InputDataTable;
} // namespace Neon::Input

struct GLFWwindow;

NEON_SIGNAL_DECL(OnWindowSizeChanged, const Neon::Size2I& /*NewSize*/);
NEON_SIGNAL_DECL(OnWindowMinized, bool /*Minimized*/);
NEON_SIGNAL_DECL(OnWindowClosed);
NEON_SIGNAL_DECL(OnWindowTitleHitTest, const Vector2I& /*MousePos*/, bool& /*WasHit*/);

namespace Neon::Windowing
{
    class WindowApp
    {
    public:
        WindowApp(
            const WindowInitDesc& Desc);

        NEON_CLASS_NO_COPY(WindowApp);

        WindowApp(
            WindowApp&& Other) noexcept :
            m_Handle(std::exchange(Other.m_Handle, nullptr)),
            m_WindowSize(Other.m_WindowSize),
            m_InputTables(std::move(Other.m_InputTables))
        {
        }

        WindowApp& operator=(
            WindowApp&& Other) noexcept
        {
            if (this != &Other)
            {
                m_Handle      = std::exchange(Other.m_Handle, nullptr);
                m_WindowSize  = Other.m_WindowSize;
                m_InputTables = std::move(Other.m_InputTables);
            }

            return *this;
        }

        ~WindowApp();

        /// <summary>
        /// Get window handle
        /// </summary>
        [[nodiscard]] GLFWwindow* GetHandle() const;

        /// <summary>
        /// Close the window
        /// The window will remain valid until its destructor is called
        /// </summary>
        void Close();

        /// <summary>
        /// Check whether the window is running or not
        /// </summary>
        [[nodiscard]] bool IsRunning() const;

        /// <summary>
        /// Get window title
        /// </summary>
        [[nodiscard]] const StringU8& GetTitle() const;

        /// <summary>
        /// Set window title
        /// </summary>
        void SetTitle(
            StringU8 Title);

        /// <summary>
        /// Get window position
        /// </summary>
        [[nodiscard]] Vector2I GetPosition() const;

        /// <summary>
        /// Set window position
        /// </summary>
        void SetPosition(
            const Vector2I& Position);

        /// <summary>
        /// Get window size
        /// </summary>
        [[nodiscard]] Size2I GetSize() const;

        /// <summary>
        /// Set window size
        /// </summary>
        void SetSize(
            const Size2I& Size);

        /// <summary>
        /// Set window to fullscreen
        /// </summary>
        void SetFullscreen(
            bool State);

        /// <summary>
        /// Check whether the window is minimized or not
        /// </summary>
        [[nodiscard]] bool IsMinimized() const;

        /// <summary>
        /// Minimize the window.
        /// </summary>
        void Minimize();

        /// <summary>
        /// Restore the window.
        /// </summary>
        void Restore();

        /// <summary>
        /// Check whether the window is maximized or not
        /// </summary>
        [[nodiscard]] bool IsMaximized() const;

        /// <summary>
        /// Maximize the window.
        /// </summary>
        void Maximize();

        /// <summary>
        /// Check whether the window is maximized or not
        /// </summary>
        [[nodiscard]] bool IsFullScreen() const;

        /// <summary>
        /// Check whether the window is visible or not
        /// </summary>
        [[nodiscard]] bool IsVisible() const;

        /// <summary>
        /// Set window icon
        /// </summary>
        void SetIcon(
            void*           IconData,
            const Vector2I& Size);

        /// <summary>
        /// Set window visbility
        /// </summary>
        void SetVisible(
            bool Show);

        /// <summary>
        /// Set the current window to be made the active foreground window
        /// </summary>
        void RequestFocus();

        /// <summary>
        /// Set the current window to be made the active foreground window
        /// </summary>
        [[nodiscard]] bool HasFocus() const;

    public:
        /// <summary>
        /// Enable input table for the window to receive input events
        /// </summary>
        void RegisterInputTable(
            const Ptr<Input::InputDataTable>& InputTable);

        /// <summary>
        /// Disable input table for the window
        /// </summary>
        void UnregisterInputTable(
            const Ptr<Input::InputDataTable>& InputTable);

        /// <summary>
        /// Process window events
        /// </summary>
        void ProcessEvents();

    public:
        /// <summary>
        /// Event fired when the window is resized
        /// </summary>
        NEON_SIGNAL_INST(OnWindowSizeChanged);

        /// <summary>
        /// Event fired when the window is minimized or restored
        /// </summary>
        NEON_SIGNAL_INST(OnWindowMinized);

        /// <summary>
        /// Event fired when the window is closed
        /// </summary>
        NEON_SIGNAL_INST(OnWindowClosed);

        /// <summary>
        /// Event fired when the window title bar is hit
        /// </summary>
        NEON_SIGNAL_INST(OnWindowTitleHitTest);

    private:
        /// <summary>
        /// Setup window inputs
        /// </summary>
        void SetupInputs();

    private:
        GLFWwindow* m_Handle;
        Size2I      m_WindowSize;
        StringU8    m_Title;

        std::unordered_set<Ptr<Input::InputDataTable>> m_InputTables;
    };
} // namespace Neon::Windowing