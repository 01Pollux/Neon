#pragma once

#include <Core/Neon.hpp>
#include <Math/Matrix.hpp>
#include <Math/Colors.hpp>

namespace Neon::RHI
{
    class IBuffer;
    class IGraphicsCommandList;
} // namespace Neon::RHI

namespace Neon::Runtime
{
    class DebugOverlay
    {
    public:
        [[nodiscard]] NEON_M_DEBUG_ONLY(static void Create());
        [[nodiscard]] NEON_M_DEBUG_ONLY(static void Destroy());

        DebugOverlay() = default;

        NEON_CLASS_NO_COPYMOVE(DebugOverlay);

        virtual ~DebugOverlay() = default;

        /// <summary>
        /// Flush all draw calls into ommand list.
        /// </summary>
        static void Render(
            RHI::IGraphicsCommandList* CommandList,
            RHI::IBuffer*              PerFrameData)
        {
#ifndef NEON_DIST
            s_DebugOverlay->Render_Impl(CommandList, PerFrameData);
#endif
        }

        /// <summary>
        /// Draw a line in world space.
        /// </summary>
        static void DrawLine(
            const Vector3& StartPosition,
            const Vector3& EndPosition,
            const Color4&  StartColor,
            const Color4&  EndColor)
        {
#ifndef NEON_DIST
            s_DebugOverlay->DrawLine_Impl(StartPosition, EndPosition, StartColor, EndColor);
#endif
        }

        /// <summary>
        /// Draw a cuboid line in world space.
        /// </summary>
        static void DrawCuboidLine(
            const Vector3& CenterPosition,
            const Vector3& Size,
            const Color4&  Color)
        {
#ifndef NEON_DIST
            s_DebugOverlay->DrawCuboidLine_Impl(CenterPosition, Size, Color);
#endif
        }

    protected:
        NEON_M_DEBUG_ONLY_PV(void Render_Impl(
            RHI::IGraphicsCommandList* CommandList,
            RHI::IBuffer*              PerFrameData));

        NEON_M_DEBUG_ONLY_PV(void DrawLine_Impl(
            const Vector3& StartPosition,
            const Vector3& EndPosition,
            const Color4&  StartColor,
            const Color4&  EndColor));

        NEON_M_DEBUG_ONLY_PV(void DrawCuboidLine_Impl(
            const Vector3& CenterPosition,
            const Vector3& Size,
            const Color4&  Color));

    private:
#ifndef NEON_DIST
        static inline DebugOverlay* s_DebugOverlay = nullptr;
#endif
    };
} // namespace Neon::Runtime
