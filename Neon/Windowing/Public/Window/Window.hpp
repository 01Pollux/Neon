#pragma once

#include <Window/Event.hpp>
#include <Core/BitMask.hpp>

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
    using MWindowStyle = BitMask<EWindowStyle>;

    static constexpr MWindowStyle s_DefaultWindowStyle = BitMask_Or(
        EWindowStyle::TitleBar,
        EWindowStyle::Resize,
        EWindowStyle::Close);

    class NEON_NOVTABLE IWindowApp
    {
    public:
        [[nodiscard]] static IWindowApp* Create(
            const String&       Title,
            const Size2I&       Size,
            const MWindowStyle& Style);

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
        /// Get window style
        /// </summary>
        [[nodiscard]] virtual MWindowStyle GetStyle() const = 0;

        /// <summary>
        /// Set window style
        /// </summary>
        virtual void SetStyle(
            const MWindowStyle& Style) = 0;

        /// <summary>
        /// Get window screen caps
        /// </summary>
        [[nodiscard]] virtual Size2I GetScreenCaps() const = 0;

        /// <summary>
        /// Get window position
        /// </summary>
        [[nodiscard]] virtual Vector2DI GetPosition() const = 0;

        /// <summary>
        /// Set window position
        /// </summary>
        virtual void SetPosition(
            const Vector2DI& Position) = 0;

        /// <summary>
        /// Get window size
        /// </summary>
        [[nodiscard]] virtual Size2I GetSize() const = 0;

        /// <summary>
        /// Set window size
        /// </summary>
        virtual void SetSize(
            const Size2I& Size) = 0;

        /// <summary>
        /// Set window icon
        /// </summary>
        virtual void SetIcon(
            const StringU8& IconPath) = 0;

        /// <summary>
        /// Set window icon
        /// </summary>
        virtual void SetIcon(
            const void*      IconData,
            const Vector2DI& Size) = 0;

        /// <summary>
        /// Set window visbility
        /// </summary>
        virtual void SetVisible(
            bool Show) = 0;

        /// <summary>
        /// Get window visbility
        /// </summary>
        [[nodiscard]] virtual bool IsVisible() const = 0;

        /// <summary>
        /// Set the current window to be made the active foreground window
        /// </summary>
        virtual void Focus(
            bool Show) = 0;

        /// <summary>
        /// Set the current window to be made the active foreground window
        /// </summary>
        [[nodiscard]] virtual bool HasFocus() const = 0;

        /// <summary>
        /// Peek event with option to erase it from the queue
        /// return true if event was found, false otherwise
        /// </summary>
        [[nodiscard]] virtual bool PeekEvent(
            Event* Msg,
            bool   Erase = true,
            bool   Block = false) = 0;
    };
} // namespace Neon::Windowing