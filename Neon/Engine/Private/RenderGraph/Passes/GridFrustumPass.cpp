#include <EnginePCH.hpp>

#include <Math/Frustum.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/GridFrustumPass.hpp>

#include <RHI/PipelineState.hpp>

namespace Neon::RG
{
    GridFrustumPass::GridFrustumPass() :
        RenderPass("GridFrustumPass")
    {
    }

    void GridFrustumPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        /* auto Desc = RHI::ResourceDesc::Buffer(
             Resolver.GetSwapchainFormat(),
             0, 0, 1, 1);

         const ResourceId GridFrustum("GridFrustum");

         Resolver.CreateWindowTexture(
             GridFrustum,
             Desc);*/
    }

    void GridFrustumPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        // RecreateGridFrustum(Storage.GetOutputImageSize());
    }

    void GridFrustumPass::RecreateGridFrustum(
        const Size2I& OutputSize)
    {
        auto& GroupSize = m_GridFrustumPipelineState->GetComputeGroupSize();

        Size2I GroupCount(
            int(Math::DivideByMultiple(OutputSize.x, GroupSize.x)),
            int(Math::DivideByMultiple(OutputSize.y, GroupSize.y)));

        size_t SizeInBytes = GroupCount.x * GroupCount.y * sizeof(Geometry::Frustum);

        bool Recreate = (SizeInBytes != m_GridFrustum->GetDesc().Width);
        if (!Recreate)
        {
            return;
        }

        RHI::MResourceFlags Flags;
        Flags.Set(RHI::EResourceFlags::AllowUnorderedAccess);

        m_GridFrustum.reset(RHI::IGpuResource::Create(
            RHI::ResourceDesc::Buffer(
                SizeInBytes,
                Flags)));
    }
} // namespace Neon::RG