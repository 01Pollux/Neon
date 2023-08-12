#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/GBufferPass.hpp>

namespace Neon::RG
{
    using namespace Scene;
    using namespace Renderer;

    GBufferPass::GBufferPass() :
        IRenderPass(STR("GBufferPass"), PassQueueType::Direct)
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
                    // TODO: Until we have skybox pass, we will use white color
                    //.Value  = Colors::Black
                    .Value = Colors::LightGray
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
    }

    void GBufferPass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        auto CameraStorage = Storage.GetFrameDataHandle();
        for (auto& Renderer : m_Renderers)
        {
            Renderer->Render(CameraStorage, RenderCommandList);
        }
    }
} // namespace Neon::RG