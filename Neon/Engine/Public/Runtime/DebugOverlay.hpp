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
        struct LineArgs
        {
            Vector3  StartPosition;
            Color4U8 StartColor;
            Vector3  EndPosition;
            Color4U8 EndColor;
        };

        struct CubeArgs
        {
            Vector3  CenterPosition;
            Color4U8 Color;
            Vector3  Size;
        };

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
        /// Reset all draw calls.
        /// </summary>
        static void Reset()
        {
#ifndef NEON_DIST
            s_DebugOverlay->Reset_Impl();
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
            const LineArgs& Args,
            bool            WorldsSpace = true)
        {
#ifndef NEON_DIST
            s_DebugOverlay->DrawLine_Impl(Args, WorldsSpace);
#endif
        }

        /// <summary>
        /// Draw a line in world space.
        /// </summary>
        static void DrawLine(
            float           Duration,
            const LineArgs& Args,
            bool            WorldsSpace = true)
        {
#ifndef NEON_DIST
            s_DebugOverlay->DrawLine_Impl(Duration, Args, WorldsSpace);
#endif
        }

        /// <summary>
        /// Draw a cuboid line in world space.
        /// </summary>
        static void DrawCuboidLine(
            const CubeArgs& Args,
            bool            WorldsSpace = true)
        {
#ifndef NEON_DIST
            s_DebugOverlay->DrawCuboidLine_Impl(-1.f, Args, WorldsSpace);
#endif
        }

        /// <summary>
        /// Draw a cuboid line in world space.
        /// </summary>
        static void DrawCuboidLine(
            float           Duration,
            const CubeArgs& Args,
            bool            WorldsSpace = true)
        {
#ifndef NEON_DIST
            s_DebugOverlay->DrawCuboidLine_Impl(Duration, Args, WorldsSpace);
#endif
        }

    private:
#ifndef NEON_DIST
        void DrawCuboidLine_Impl(
            float           Duration,
            const CubeArgs& Args,
            bool            WorldsSpace);
#endif

    protected:
        NEON_M_NODIST_PV(bool ShouldRender_Impl());

        NEON_M_NODIST_PV(void Reset_Impl());

        NEON_M_NODIST_PV(void Render_Impl(
            RHI::IGraphicsCommandList* CommandList,
            RHI::GpuResourceHandle     PerFrameData));

        NEON_M_NODIST_PV(void DrawLine_Impl(
            const LineArgs& Args,
            bool            WorldsSpace));

        NEON_M_NODIST_PV(void DrawLine_Impl(
            float           Duration,
            const LineArgs& Args,
            bool            WorldsSpace));

    private:
#ifndef NEON_DIST
        static inline DebugOverlay* s_DebugOverlay = nullptr;
#endif
    };
} // namespace Neon::Runtime
