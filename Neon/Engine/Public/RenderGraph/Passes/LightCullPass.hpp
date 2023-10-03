#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class LightCullPass : public ComputeRenderPass<LightCullPass>
    {
        friend class RenderPass;

    public:
        struct LightInfo
        {
            uint32_t LightCount;
        };

        struct LightResult
        {
            uint32_t OpaqueCount;
            uint32_t TransparentCount;
        };

        // For the light index list, we need to make a guess as to the average
        // number of overlapping lights per tile. It could be possible to refine this
        // value at runtime (if it is underestimated) but for now, I'll just take a guess
        // of about 200 lights (which may be an overestimation, but better over than under).
        // The total size of the buffer will be determined by the grid size but for 16x16
        // tiles at 1080p, we would need 120x68 tiles * 256 light indices * 4 bytes (to store a uint)
        // making the light index list 8,355,840 bytes (8 MB).
        static constexpr uint32_t MaxOverlappingLightsPerTile = 256;

    public:
        LightCullPass();

        ~LightCullPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

    private:
        /// <summary>
        /// Recreates the grid frustum resource if needed. (window size changed)
        /// </summary>
        void RecreateGridFrustumIfNeeded(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

        /// <summary>
        /// Recreates the grid frustum resource.
        /// </summary>
        void RecreateGridFrustum(
            const Size2I& GridCount);

        /// <summary>
        /// Dispatches the grid frustum pass.
        /// </summary>
        void DispatchGridFrustum(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList,
            const Size2I&           GridCount);

    private:
        /// <summary>
        /// Recreates the light grid resource if needed. (window size changed)
        /// </summary>
        void RecreateLightGrid(
            const Size2I& GridCount);

        /// <summary>
        /// Create resources such as light grid, light frustum, etc.
        /// </summary>
        void CreateResources();

    private:
        Ptr<RHI::IGpuResource> m_LightIndexList_Opaque;
        Ptr<RHI::IGpuResource> m_LightIndexList_Transparent;

        Ptr<RHI::IGpuResource> m_LightGrid_Opaque;
        Ptr<RHI::IGpuResource> m_LightGrid_Transparent;

        Size2I                    m_GridSize;
        UPtr<RHI::IGpuResource>   m_GridFrustum;
        RHI::DescriptorHeapHandle m_GridFrustumViews; // 0: UAV, 1: SRV
        Ptr<RHI::IRootSignature>  m_GridFrustumRS;
        Ptr<RHI::IPipelineState>  m_GridFrustumPSO;

        Ptr<RHI::IRootSignature> m_LightCullRS;
        Ptr<RHI::IPipelineState> m_LightCullPSO;
    };
} // namespace Neon::RG