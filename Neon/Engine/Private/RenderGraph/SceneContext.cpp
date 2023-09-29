#include <EnginePCH.hpp>
#include <RenderGraph/SceneContext.hpp>
#include <RenderGraph/Storage.hpp>

#include <Runtime/GameLogic.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/GPUScene.hpp>

#include <RHI/Material/Material.hpp>
#include <RHI/Resource/Views/Shader.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::RG
{
    SceneContext::SceneContext(
        const GraphStorage& Storage) :
        m_Storage(Storage),
        m_MeshQuery(
            Scene::EntityWorld()
                .Get()
                .query_builder<
                    const Scene::Component::Transform,
                    const Scene::Component::MeshInstance,
                    const Scene::Component::Renderable>()
                .with<Component::ActiveSceneEntity>()
                .build())

    {
    }

    void SceneContext::Update()
    {
        UpdateInstances();
    }

    //

    struct PerMaterialData
    {
        Vector3  Albedo;
        uint32_t AlbedoMapIndex;

        Vector3  Specular;
        uint32_t SpecularMapIndex;

        Vector3  Emissive;
        uint32_t EmissiveMapIndex;

        uint32_t NormalMapIndex;

        // MATERIAL_FLAG_*
        uint32_t Flags;
    };

    //

    void SceneContext::Render(
        RHI::ICommandList* CommandList,
        RenderType         Type) const
    {
        auto GpuScene = Runtime::GameLogic::Get()->GetGPUScene();
        for (auto& InstanceList : m_MeshInstanceIds | std::views::values)
        {
            uint32_t FirstInstanceId = InstanceList.back();
            auto     FirstMesh       = m_Meshes.at(FirstInstanceId);
            auto&    FirstMaterial   = FirstMesh->GetModel()->GetMaterial(FirstMesh->GetData().MaterialIndex);

            if (FirstMaterial->IsCompute() && Type != RenderType::RenderPass)
            {
                continue;
            }

            Ptr<RHI::IPipelineState> PipelineState;
            switch (Type)
            {
            case RenderType::DepthPrepass:
                PipelineState = FirstMaterial->GetPipelineState(RHI::IMaterial::PipelineVariant::DepthPrePass);
                break;
            case RenderType::RenderPass:
                PipelineState = FirstMaterial->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass);
                break;
            }

            FirstMaterial->SetResourceView(
                "_FrameConstant",
                m_Storage.GetFrameDataHandle());

            CommandList->SetRootSignature(!FirstMaterial->IsCompute(), FirstMaterial->GetRootSignature());
            CommandList->SetPipelineState(PipelineState);
            // FirstMaterial->BindSharedParams(CommandList);

            for (auto InstanceId : InstanceList)
            {
                auto  Mesh         = m_Meshes.at(InstanceId);
                auto& MeshData     = Mesh->GetData();
                auto& MeshModel    = Mesh->GetModel();
                auto& MeshMaterial = Mesh->GetModel()->GetMaterial(MeshData.MaterialIndex);

                if (MeshMaterial->IsCompute() && Type != RenderType::RenderPass)
                {
                    continue;
                }

                MeshMaterial->BindLocalParams(CommandList);

                RHI::Views::Vertex VtxView;
                VtxView.Append<Mdl::MeshVertex>(
                    MeshModel->GetVertexBuffer()->GetHandle(),
                    MeshData.VertexOffset,
                    MeshData.VertexCount);

                RHI::Views::IndexU32 IdxView(
                    MeshModel->GetIndexBuffer()->GetHandle(),
                    MeshData.IndexOffset,
                    MeshData.IndexCount);

                if (1)
                {
                    auto Buffer0 = RHI::IGpuResource::Create(
                        RHI::ResourceDesc::Buffer(
                            sizeof(Matrix4x4),
                            {},
                            RHI::GraphicsBufferType::Upload));
                    auto Buffer1 = RHI::IGpuResource::Create(
                        RHI::ResourceDesc::Buffer(
                            sizeof(PerMaterialData),
                            {},
                            RHI::GraphicsBufferType::Upload));

                    auto Ptr0 = Buffer0->Map();
                    auto Ptr1 = Buffer1->Map();

                    PerMaterialData* MaterialData = std::bit_cast<PerMaterialData*>(Ptr1);
                    *MaterialData                 = {};

                    MaterialData->Albedo = Colors::DarkGreen;

                    Matrix4x4* World = std::bit_cast<Matrix4x4*>(Ptr0);
                    *World           = glm::transpose(MeshData.Transform);

                    MeshMaterial->SetResourceView("_PerInstanceData", Buffer0->GetHandle());
                    MeshMaterial->SetResourceView("_PerMaterialData", Buffer1->GetHandle());

                    MeshMaterial->BindSharedParams(CommandList);
                }

                CommandList->SetPrimitiveTopology(MeshData.Topology);
                CommandList->SetIndexBuffer(IdxView);
                CommandList->SetVertexBuffer(0, VtxView);
                CommandList->Draw(RHI::DrawIndexArgs{ .IndexCountPerInstance = MeshData.IndexCount });
            }
        }
    }

    void SceneContext::UpdateInstances()
    {
        if (!m_MeshQuery.changed())
        {
            return;
        }

        m_MeshInstanceIds.clear();
        m_Meshes.clear();

        m_MeshQuery.iter(
            [this](flecs::iter& Iter,
                   const Scene::Component::Transform*,
                   const Scene::Component::MeshInstance* MeshTable,
                   const Scene::Component::Renderable*   RenderableTable)
            {
                for (size_t Idx : Iter)
                {
                    auto& Mesh       = MeshTable[Idx].Mesh;
                    auto& Renderable = RenderableTable[Idx];

                    auto& MeshData      = Mesh.GetData();
                    auto& MeshMaterial  = Mesh.GetModel()->GetMaterial(MeshData.MaterialIndex);
                    auto& PipelineState = MeshMaterial->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass);

                    m_MeshInstanceIds[PipelineState.get()].emplace_back(Renderable.GetInstanceId());
                    m_Meshes[Renderable.GetInstanceId()] = &Mesh;
                }
            });
    }
} // namespace Neon::RG