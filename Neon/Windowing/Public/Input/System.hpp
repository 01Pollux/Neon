#pragma once

#include <Input/Types.hpp>
#include <Math/Rect.hpp>

namespace Neon::Input
{
    /// <summary>
    /// Check if a key is pressed for the current frame.
    /// </summary>
    [[nodiscard]] bool IsKeyPressed(
        EKeyboardInput Type);

    /// <summary>
    /// Check if a key is down for the current frame.
    /// </summary>
    [[nodiscard]] bool IsKeyDown(
        EKeyboardInput Type);

    /// <summary>
    /// Check if a key is released for the current frame.
    /// </summary>
    [[nodiscard]] bool IsKeyUp(
        EKeyboardInput Type);

    //

    /// <summary>
    /// Check if a mouse button is pressed for the current frame.
    /// </summary>
    [[nodiscard]] bool IsMouseDown(
        EMouseInput Type);

    /// <summary>
    /// Check if a mouse button is pressed for the current frame.
    /// </summary>
    [[nodiscard]] bool IsAnyMouseDown();

    /// <summary>
    /// Check if a mouse button is pressed for the current frame.
    /// </summary>
    [[nodiscard]] bool IsMouseUp(
        EMouseInput Type);

    /// <summary>
    /// Check if mouse is clicked, Same as GetMouseClickedCount() == 1
    /// </summary>
    [[nodiscard]] uint32_t GetMouseClickedCount(
        EMouseInput Type);

    /// <summary>
    /// Check if mouse is clicked, Same as GetMouseClickedCount() == 1
    /// </summary>
    [[nodiscard]] bool IsMouseClicked(
        EMouseInput Type,
        bool        Repeat = false);

    /// <summary>
    /// Check if mouse is clicked, Same as GetMouseClickedCount() == 2
    /// </summary>
    [[nodiscard]] bool IsMouseDoubleClicked(
        EMouseInput Type);

    /// <summary>
    /// Check if mouse is hovering over a rect.
    /// </summary>
    [[nodiscard]] bool IsMouseHoveringRect(
        const RectF& Rect);

    /// <summary>
    /// Check if mouse is hovering over a rect.
    /// </summary>
    [[nodiscard]] Vector2 GetMousePos();

    /// <summary>
    /// Check if mouse is hovering over a rect.
    /// </summary>
    [[nodiscard]] Vector2 GetMouseDelta();
} // namespace Neon::Input