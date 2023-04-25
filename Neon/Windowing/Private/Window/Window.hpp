#pragma once

#include <Window/Window.hpp>
#include <Private/Window/WindowHeaders.hpp>

#include <queue>

namespace Neon::Windowing
{
    enum class EWindowFlags : uint8_t
    {
        Resizing,

        _Last_Enum
    };
    using MWindowFlags = BitMask<EWindowFlags>;

    class WindowApp final : public IWindowApp
    {
    public:
        WindowApp(
            const String&       Title,
            const Size2I&       Size,
            const MWindowStyle& Style);

        NEON_CLASS_NO_COPYMOVE(WindowApp);

        ~WindowApp() override;

        [[nodiscard]] void* GetPlatformHandle() const override;

        void Close() override;

        [[nodiscard]] MWindowStyle GetStyle() const override;

        void SetStyle(
            const MWindowStyle& Style) override;

        [[nodiscard]] Size2I GetScreenCaps() const override;

        [[nodiscard]] Vector2DI GetPosition() const override;

        void SetPosition(
            const Vector2DI& Position) override;

        [[nodiscard]] Size2I GetSize() const override;

        void SetSize(
            const Size2I& Size) override;

        void SetIcon(
            const StringU8& IconPath) override;

        void SetIcon(
            const void*      IconData,
            const Vector2DI& Size) override;

        void SetVisible(
            bool Show) override;

        [[nodiscard]] bool IsVisible() const override;

        void Focus(
            bool Show) override;

        [[nodiscard]] bool HasFocus() const override;

        [[nodiscard]] bool PeekEvent(
            Event* Msg,
            bool   Erase,
            bool   Block) override;

    public:
        static LRESULT WndProc(
            HWND   Handle,
            UINT   Message,
            WPARAM wParam,
            LPARAM lParam);

        static LRESULT WndSubClassProc(
            HWND      Handle,
            UINT      Message,
            WPARAM    wParam,
            LPARAM    lParam,
            UINT_PTR  IdSubclass,
            DWORD_PTR RefData);

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
        HWND m_Handle = nullptr;

        Size2I m_WindowSize;
        Size2I m_UncappedWindowSize;
        int    m_BitsPerPixel;

        std::queue<Event> m_PendingEvents;
        MWindowFlags      m_WindowFlags;
        MWindowStyle      m_WindowStyle;
    };
} // namespace Neon::Windowing