#pragma once

#include <RHI/Resource/Common.hpp>
#include <Core/Neon.hpp>
#include <Math/Matrix.hpp>
#include <Math/Colors.hpp>

namespace Neon::RHI
{
    class IGraphicsCommandList;
} // namespace Neon::RHI

namespace Neon::Runtime
{
    class DebugOverlay
    {
    public:
        NEON_M_NODIST(static void Create());
        NEON_M_NODIST(static void Destroy());

        DebugOverlay() = default;

        NEON_CLASS_NO_COPYMOVE(DebugOverlay);

        virtual ~DebugOverlay() = default;

        /// <summary>
        /// Check if debug overlay should be drawn.
        /// </summary>
        [[nodiscard]] static bool ShouldRender()
        {
#ifndef NEON_DIST
            return s_DebugOverlay->ShouldRender_Impl();
#endif
        }

        /// <summary>
        /// Flush all draw calls into ommand list.
        /// </summary>
        static void Render(
            RHI::IGraphicsCommandList* CommandList,
            RHI::GpuResourceHandle     PerFrameData)
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
        NEON_M_NODIST_PV(bool ShouldRender_Impl());

        NEON_M_NODIST_PV(void Render_Impl(
            RHI::IGraphicsCommandList* CommandList,
            RHI::GpuResourceHandle     PerFrameData));

        NEON_M_NODIST_PV(void DrawLine_Impl(
            const Vector3& StartPosition,
            const Vector3& EndPosition,
            const Color4&  StartColor,
            const Color4&  EndColor));

        NEON_M_NODIST_PV(void DrawCuboidLine_Impl(
            const Vector3& CenterPosition,
            const Vector3& Size,
            const Color4&  Color));

    private:
#ifndef NEON_DIST
        static inline DebugOverlay* s_DebugOverlay = nullptr;
#endif
    };
} // namespace Neon::Runtime