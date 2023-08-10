#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/GBufferPass.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>

namespace Neon::RG
{
    using namespace Scene;
    using namespace Renderer;

    GBufferPass::GBufferPass(
        flecs::entity Camera) :
        IRenderPass("GBufferPass", PassQueueType::Direct),
        m_Camera(Camera)
    {
    }

    void GBufferPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        auto MakeResourceDesc =
            [](size_t Index,
               bool   IsDepthStencil = false) -> RHI::ResourceDesc
        {
            auto Desc = RHI::ResourceDesc::Tex2D(
                RenderTargetsFormats[Index],
                0, 0, 1, 1);

            if (IsDepthStencil)
            {
                Desc.ClearValue = RHI::ClearOperation{
                    .Format = RenderTargetsFormatsTyped[Index],
                    .Value  = RHI::ClearOperation::DepthStencil{
                         .Depth = 1.f },
                };
            }
            else
            {
                Desc.ClearValue = RHI::ClearOperation{
                    .Format = RenderTargetsFormatsTyped[Index],
                    .Value  = Colors::Black
                };
            }

            return Desc;
        };

        std::array RenderTargets{
            std::pair{ ResourceId(STR("GBufferAlbedo")), MakeResourceDesc(0) },
            std::pair{ ResourceId(STR("GBufferNormal")), MakeResourceDesc(1) },
            std::pair{ ResourceId(STR("GBufferEmissive")), MakeResourceDesc(2) },

            std::pair{ ResourceId(STR("GBufferDepth")), MakeResourceDesc(3, true) },
        };
        static_assert(RenderTargets.size() == std::size(RenderTargetsFormats), "RenderTargetsFormats and RenderTargets must have the same size");

        for (size_t i = 0; i < RenderTargets.size(); i++)
        {
            auto& [Resource, Desc] = RenderTargets[i];

            Resolver.CreateWindowTexture(Resource, Desc);

            // Write to render target
            if (i != (RenderTargets.size() - 1)) [[likely]]
            {
                Resolver.WriteRenderTarget(
                    Resource.CreateView(STR("Main")),
                    RHI::RTVDesc{
                        .View      = RHI::RTVDesc::Texture2D{},
                        .ClearType = RHI::ERTClearType::Color,
                        .Format    = Desc.ClearValue->Format });
            }
            // Write to depth buffer
            else
            {
                Resolver.WriteDepthStencil(
                    Resource.CreateView(STR("Main")),
                    RHI::DSVDesc{
                        .View      = RHI::DSVDesc::Texture2D{},
                        .ClearType = RHI::EDSClearType::Depth,
                        .Format    = Desc.ClearValue->Format });
            }
        }

        ResourceId CameraBufferId(STR("CameraBuffer"));
        Resolver.CreateBuffer(
            CameraBufferId,
            RHI::BufferDesc{
                .Size      = sizeof(Component::CameraFrameData),
                .Alignment = 255,
                .UsePool   = false },
            RHI::GraphicsBufferType::Upload);
    }

    void GBufferPass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        // auto CameraStorage = UpdateCameraBuffer();
        RHI::GpuResourceHandle CameraStorage;
        ResourceId             CameraBufferId(STR("CameraBuffer"));
        auto                   Buffer = Storage.GetResource(CameraBufferId).AsUploadBuffer();
        {
            auto CameraComponent = m_Camera.get<Component::Camera>();
            auto CameraTransform = m_Camera.get<Component::Transform>();

            auto CameraBuffer = Buffer->Map<Component::CameraFrameData>();

            CameraBuffer->World = glm::transpose(CameraTransform->World.ToMat4x4());

            CameraBuffer->View           = glm::transpose(CameraComponent->ViewMatrix(m_Camera));
            CameraBuffer->Projection     = glm::transpose(CameraComponent->ProjectionMatrix());
            CameraBuffer->ViewProjection = CameraBuffer->View * CameraBuffer->Projection;

            CameraBuffer->ViewInverse           = glm::inverse(CameraBuffer->View);
            CameraBuffer->ProjectionInverse     = glm::inverse(CameraBuffer->Projection);
            CameraBuffer->ViewProjectionInverse = glm::inverse(CameraBuffer->ViewProjection);

            CameraBuffer->EngineTime = float(Runtime::DefaultGameEngine::Get()->GetEngineTime());
            CameraBuffer->GameTime   = float(Runtime::DefaultGameEngine::Get()->GetGameTime());
            CameraBuffer->DeltaTime  = float(Runtime::DefaultGameEngine::Get()->GetDeltaTime());

            Buffer->Unmap();
            CameraStorage = Buffer->GetHandle();
        }

        for (auto& Renderer : m_Renderers)
        {
            Renderer->Render(CameraStorage, RenderCommandList);
        }
    }

    RHI::GpuResourceHandle GBufferPass::UpdateCameraBuffer()
    {
        auto CameraComponent = m_Camera.get<Component::Camera>();
        auto CameraTransform = m_Camera.get<Component::Transform>();
        auto CameraBuffer    = CameraComponent->GraphicsBuffer->Map<Component::CameraFrameData>();

        CameraBuffer->World = glm::transpose(CameraTransform->World.ToMat4x4());

        CameraBuffer->View           = glm::transpose(CameraComponent->ViewMatrix(m_Camera));
        CameraBuffer->Projection     = glm::transpose(CameraComponent->ProjectionMatrix());
        CameraBuffer->ViewProjection = CameraBuffer->View * CameraBuffer->Projection;

        CameraBuffer->ViewInverse           = glm::inverse(CameraBuffer->View);
        CameraBuffer->ProjectionInverse     = glm::inverse(CameraBuffer->Projection);
        CameraBuffer->ViewProjectionInverse = glm::inverse(CameraBuffer->ViewProjection);

        CameraBuffer->EngineTime = float(Runtime::DefaultGameEngine::Get()->GetEngineTime());
        CameraBuffer->GameTime   = float(Runtime::DefaultGameEngine::Get()->GetGameTime());
        CameraBuffer->DeltaTime  = float(Runtime::DefaultGameEngine::Get()->GetDeltaTime());

        CameraComponent->GraphicsBuffer->Unmap();
        return CameraComponent->GraphicsBuffer->GetHandle();
    }
} // namespace Neon::RG