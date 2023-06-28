#pragma once

#include <Window/Window.hpp>
#include <Private/Window/WindowHeaders.hpp>
#include <queue>

namespace Neon::Windowing
{
    enum class EWindowFlags : uint8_t
    {
        Resizing,
        Minimized,
        Maximized,

        _Last_Enum
    };
    using MWindowFlags = Bitmask<EWindowFlags>;

    class WindowApp final : public IWindowApp
    {
    public:
        WindowApp(
            const String&       Title,
            const Size2I&       Size,
            const MWindowStyle& Style,
            bool                StartInMiddle);

        NEON_CLASS_NO_COPYMOVE(WindowApp);

        ~WindowApp() override;

        [[nodiscard]] void* GetPlatformHandle() const override;

        void Close() override;

        String GetTitle() const override;

        /// <summary>
        /// Set window title
        /// </summary>
        void SetTitle(
            const String& Title) override;

        [[nodiscard]] MWindowStyle GetStyle() const override;

        void SetStyle(
            const MWindowStyle& Style) override;

        [[nodiscard]] Size2I GetScreenCaps() const override;

        [[nodiscard]] Vector2I GetPosition() const override;

        void SetPosition(
            const Vector2I& Position) override;

        [[nodiscard]] Size2I GetSize() const override;

        void SetSize(
            const Size2I& Size) override;

        bool IsMinimized() const;

        bool IsMaximized() const;

        bool IsVisible() const;

        void SetIcon(
            const void*     IconData,
            const Vector2I& Size) override;

        void SetVisible(
            bool Show) override;

        void RequestFocus() override;

        [[nodiscard]] bool HasFocus() const override;

        [[nodiscard]] bool PeekEvent(
            Event* Msg,
            bool   Erase,
            bool   Block) override;

    public:
        /// <summary>
        /// Window thread function that creates the window and processes messages.
        /// </summary>
        void WindowThread();

        /// <summary>
        /// Window procedure that handles all messages sent to the window.
        /// </summary>
        static LRESULT WndProc(
            HWND   Handle,
            UINT   Message,
            WPARAM wParam,
            LPARAM lParam);

    private:
        /// <summary>
        /// Get window style from MWindowStyle
        /// </summary>
        [[nodiscard]] static DWORD GetWindowStyle(
            const MWindowStyle& Flags);

        /// <summary>
        /// Switch to fullscreen mode.
        /// </summary>
        void SwitchToFullscreen();

        /// <summary>
        /// Process all events in the event queue.
        /// </summary>
        void ProcessMessages();

        /// <summary>
        /// Emplace an event into the event queue.
        /// </summary>
        void QueueEvent(
            Event Msg);

        void ProcessMessage(
            UINT   Message,
            WPARAM wParam,
            LPARAM lParam);

    private:
        HWND  m_Handle = nullptr;
        HICON m_Icon   = nullptr;

        Size2I m_WindowSize;
        Size2I m_UncappedWindowSize;
        int    m_BitsPerPixel;

        std::queue<Event> m_PendingEvents;
        MWindowFlags      m_WindowFlags;
        MWindowStyle      m_WindowStyle;

        std::jthread m_WindowThread;
    };
} // namespace Neon::Windowing