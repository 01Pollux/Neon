#include <EnginePCH.hpp>
#include <Renderer/Render/MeshRenderer.hpp>
#include <Scene/EntityWorld.hpp>


namespace Component = Neon::Scene::Component;

namespace Neon::Renderer
{
    /// <summary>
    /// Update and apply the material to command list
    /// </summary>
    static void ApplyMeshMaterial(
        MeshRenderer::FrameResource& FrameResource,
        size_t                       SubresourceIndex,
        RHI::IMaterial*              Material,
        RHI::GpuResourceHandle       CameraBuffer,
        const Component::Transform&  Transform,
        RHI::ICommandList*           CommandList);

    //

    MeshRenderer::MeshRenderer() 
        //:
        //m_MeshQuery(
        //    Scene::EntityWorld::Get()
        //        .query_builder<Component::Transform, Component::MeshInstance>()
        //        .with<Component::ActiveSceneEntity>()
        //        .term<Component::Transform>()
        //        .in()
        //        .term<Component::MeshInstance>()
        //        .in()
        //        // Order by material's pipeline state
        //        .order_by<Component::MeshInstance>(
        //            [](flecs::entity_t                EntLhs,
        //               const Component::MeshInstance* MeshLhs,
        //               flecs::entity_t                EntRhs,
        //               const Component::MeshInstance* MeshRhs) -> int32_t
        //            {
        //                auto& ModelLhs = MeshLhs->Mesh.GetModel();
        //                auto& ModelRhs = MeshRhs->Mesh.GetModel();

        //                auto MatIdxLhs = MeshLhs->Mesh.GetSubmesh().MaterialIndex;
        //                auto MatIdxRhs = MeshRhs->Mesh.GetSubmesh().MaterialIndex;

        //                auto& MaterialLhs = ModelLhs->GetMaterial(MatIdxLhs);
        //                auto& MaterialRhs = ModelRhs->GetMaterial(MatIdxRhs);

        //                auto& PipelineStateLhs = MaterialLhs->GetPipelineState();
        //                auto& PipelineStateRhs = MaterialRhs->GetPipelineState();

        //                if (PipelineStateLhs < PipelineStateRhs)
        //                {
        //                    return -1;
        //                }
        //                else if (PipelineStateLhs > PipelineStateRhs)
        //                {
        //                    return 1;
        //                }
        //                return 0;
        //            })
        //        .build())
    {
    }

    MeshRenderer::~MeshRenderer()
    {
        m_MeshQuery.destruct();
    }

    //

    void MeshRenderer::Render(
        RHI::GpuResourceHandle CameraBuffer,
        RHI::ICommandList*     CommandList)
    {
    }

    MeshRenderer::FrameResource::FrameResource() :
        PerObjectBuffer(
            RHI::IGpuResource::Create(
                RHI::ResourceDesc::BufferUpload(sizeof(MeshRenderer::PerObjectData) * 1024))),
        PerMaterialBuffer(
            RHI::IGpuResource::Create(
                RHI::ResourceDesc::BufferUpload(sizeof(MeshRenderer::PerMaterialData) * 1024)))
    {
    }
} // namespace Neon::Renderer