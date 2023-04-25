#pragma once

#include <Window/Window.hpp>
#include <Private/Window/WindowHeaders.hpp>

namespace Neon::Windowing
{
    class WindowApp final : public IWindowApp
    {
    public:
        WindowApp(
            const String& Title,
            const Size2I& Size,
            MWindowStyle  Style);

        ~WindowApp() override;

        [[nodiscard]] void* GetPlatformHandle() const override;

        void Close() override;

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

        [[nodiscard]] bool hasFocus() const override;

        [[nodiscard]] const Event* PeekEvent(
            bool Erase) const override;

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
        void ProcessMessage(
            UINT   Message,
            WPARAM wParam,
            LPARAM lParam);

    private:
        HWND m_Handle = nullptr;
    };
} // namespace Neon::Windowing