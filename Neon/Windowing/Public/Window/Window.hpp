#pragma once

#include <Window/Event.hpp>
#include <Core/Bitmask.hpp>
#include <Utils/Signal.hpp>
#include <future>
#include <unordered_set>

namespace Neon::Input
{
    class IInputDataTable;
} // namespace Neon::Input

namespace Neon::Windowing
{
    enum class EWindowStyle : uint8_t
    {
        TitleBar,
        Resize,
        Close,

        Windowed,
        Fullscreen,

        _Last_Enum
    };
    using MWindowStyle = Bitmask<EWindowStyle>;

    static constexpr MWindowStyle s_DefaultWindowStyle = BitMask_Or(
        EWindowStyle::TitleBar,
        EWindowStyle::Resize,
        EWindowStyle::Close);

    class IWindowApp
    {
    public:
        [[nodiscard]] static IWindowApp* Create(
            const String&       Title,
            const Size2I&       Size,
            const MWindowStyle& Style,
            bool                StartInMiddle,
            bool                InitialVisible);

        IWindowApp() = default;

        NEON_CLASS_NO_COPYMOVE(IWindowApp);

        virtual ~IWindowApp() = default;

        /// <summary>
        /// Get window handle
        /// </summary>
        [[nodiscard]] virtual void* GetPlatformHandle() const = 0;

        /// <summary>
        /// Close the window
        /// The window will remain valid until its destructor is called
        /// </summary>
        virtual void Close() = 0;

        /// <summary>
        /// Check whether the window is running or not
        /// </summary>
        [[nodiscard]] virtual bool IsRunning() const = 0;

        /// <summary>
        /// Get window title
        /// </summary>
        [[nodiscard]] virtual std::future<String> GetTitle() const = 0;

        /// <summary>
        /// Set window title
        /// </summary>
        virtual std::future<void> SetTitle(
            const String& Title) = 0;

        /// <summary>
        /// Get window style
        /// </summary>
        [[nodiscard]] virtual std::future<MWindowStyle> GetStyle() const = 0;

        /// <summary>
        /// Set window style
        /// </summary>
        virtual std::future<void> SetStyle(
            const MWindowStyle& Style) = 0;

        /// <summary>
        /// Get window screen caps
        /// </summary>
        [[nodiscard]] virtual std::future<Size2I> GetScreenCaps() const = 0;

        /// <summary>
        /// Get window position
        /// </summary>
        [[nodiscard]] virtual std::future<Vector2I> GetPosition() const = 0;

        /// <summary>
        /// Set window position
        /// </summary>
        virtual std::future<void> SetPosition(
            const Vector2I& Position) = 0;

        /// <summary>
        /// Get window size
        /// </summary>
        [[nodiscard]] virtual std::future<Size2I> GetSize() const = 0;

        /// <summary>
        /// Check whether the window is minimized or not
        /// </summary>
        [[nodiscard]] virtual std::future<bool> IsMinimized() const = 0;

        /// <summary>
        /// Check whether the window is maximized or not
        /// </summary>
        [[nodiscard]] virtual std::future<bool> IsMaximized() const = 0;

        /// <summary>
        /// Check whether the window is visible or not
        /// </summary>
        [[nodiscard]] virtual std::future<bool> IsVisible() const = 0;

        /// <summary>
        /// Set window size
        /// </summary>
        virtual std::future<void> SetSize(
            const Size2I& Size) = 0;

        /// <summary>
        /// Set window icon
        /// </summary>
        virtual std::future<void> SetIcon(
            const void*     IconData,
            const Vector2I& Size) = 0;

        /// <summary>
        /// Set window visbility
        /// </summary>
        virtual std::future<void> SetVisible(
            bool Show) = 0;

        /// <summary>
        /// Set the current window to be made the active foreground window
        /// </summary>
        virtual std::future<void> RequestFocus() = 0;

        /// <summary>
        /// Set the current window to be made the active foreground window
        /// </summary>
        [[nodiscard]] virtual std::future<bool> HasFocus() const = 0;

        /// <summary>
        /// Peek the next event from the event queue and return true if there is atleast one
        /// </summary>
        [[nodiscard]] virtual bool PeekEvent(
            Event& Message) = 0;

        /// <summary>
        /// Update imgui's docking system
        /// Used in Dx12/ImGui.cpp
        /// </summary>
        virtual void UpdateImGuiDockingSystem() = 0;

    public:
        /// <summary>
        /// Enable input table for the window to receive input events
        /// </summary>
        void PushInputTable(
            const Ptr<Input::IInputDataTable>& InputTable);

        /// <summary>
        /// Disable input table for the window
        /// </summary>
        void PopInputTable(
            const Ptr<Input::IInputDataTable>& InputTable);

        /// <summary>
        /// Process input events
        /// </summary>
        void ProcessInputs();

    protected:
        std::unordered_set<Ptr<Input::IInputDataTable>> m_InputTables;
    };
} // namespace Neon::Windowing