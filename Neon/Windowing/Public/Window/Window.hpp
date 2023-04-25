#pragma once

#include <Window/Event.hpp>
#include <Core/BitMask.hpp>

namespace Neon::Windowing
{
    enum class EWindowStyle
    {
        TitleBar,
        Resize,
        Close,
        Fullscreen,

        _Last_Enum
    };
    using MWindowStyle = BitMask<EWindowStyle>;

    static inline MWindowStyle s_DefaultWindowStyle = MWindowStyle().Flip();

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
        [[nodiscard]] virtual bool hasFocus() const = 0;

        /// <summary>
        /// Peek event with option to erase it from the queue
        /// returns nullptr if no event is available
        /// </summary>
        [[nodiscard]] virtual const Event* PeekEvent(
            bool Erase) const = 0;
    };
} // namespace Neon::Windowing