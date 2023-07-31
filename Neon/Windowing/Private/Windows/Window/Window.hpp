#pragma once

#include <Window/Window.hpp>
#include <Private/Windows/Window/WindowHeaders.hpp>
#include <Asio/QueueTask.hpp>
#include <queue>
#include <latch>

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
        static constexpr UINT WM_USER_TASK_PENDING = WM_USER + 0x7b15;

    public:
        WindowApp(
            const String&       Title,
            const Size2I&       Size,
            const MWindowStyle& Style,
            bool                StartInMiddle,
            bool                InitialVisible);

        NEON_CLASS_NO_COPYMOVE(WindowApp);

        ~WindowApp() override;

        [[nodiscard]] void* GetPlatformHandle() const override;

        void Close() override;

        bool IsRunning() const override;

        std::future<String> GetTitle() const override;

        std::future<void> SetTitle(
            const String& Title) override;

        [[nodiscard]] std::future<MWindowStyle> GetStyle() const override;

        std::future<void> SetStyle(
            const MWindowStyle& Style) override;

        [[nodiscard]] std::future<Size2I> GetScreenCaps() const override;

        [[nodiscard]] std::future<Vector2I> GetPosition() const override;

        std::future<void> SetPosition(
            const Vector2I& Position) override;

        [[nodiscard]] std::future<Size2I> GetSize() const override;

        std::future<void> SetSize(
            const Size2I& Size) override;

        std::future<bool> IsMinimized() const override;

        std::future<bool> IsMaximized() const override;

        std::future<bool> IsVisible() const override;

        std::future<void> SetIcon(
            const void*     IconData,
            const Vector2I& Size) override;

        std::future<void> SetVisible(
            bool Show) override;

        std::future<void> RequestFocus() override;

        [[nodiscard]] std::future<bool> HasFocus() const override;

        bool PeekEvent(
            Event& Message) override;

    private:
        template<typename _FnTy, typename... _Args>
        auto DispatchTask(
            _FnTy&& Task,
            _Args&&... Args) const
        {
            auto Future = m_TaskQueue.PushTask(std::forward<_FnTy>(Task), std::forward<_Args>(Args)...);
            PostTaskMessage();
            return Future;
        }

        /// <summary>
        /// Post a task message to the window thread.
        /// </summary>
        void PostTaskMessage() const;

        /// <summary>
        /// Window thread function that creates the window and processes messages.
        /// </summary>
        void WindowThread();

    public:
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
        /// Emplace an event into the event queue.
        /// </summary>
        void QueueEvent(
            Event Msg);

        /// <summary>
        /// Process a message sent to the window.
        /// </summary>
        [[nodiscard]] std::optional<LRESULT> ProcessMessage(
            UINT   Message,
            WPARAM wParam,
            LPARAM lParam);

        /// <summary>
        /// Get window size.
        /// </summary>
        [[nodiscard]] Size2I GetWindowSize() const;

        /// <summary>
        /// Get window title.
        /// </summary>
        [[nodiscard]] String GetWindowTitle() const;

    private:
        HWND  m_Handle = nullptr;
        HICON m_Icon   = nullptr;

        Size2I m_WindowSize;
        Size2I m_FullscreenSize;
        RECT   m_WindowRect;
        int    m_BitsPerPixel = 0;

        std::queue<Event> m_PendingEvents;
        std::mutex        m_PendingEventsMutex;

        MWindowFlags m_WindowFlags;
        MWindowStyle m_WindowStyle;

        std::atomic_bool          m_IsRunning = true;
        std::latch                m_WindowCreatedLatch;
        mutable Asio::QueueTaskMT m_TaskQueue;
        std::jthread              m_WindowThread;
    };
} // namespace Neon::Windowing