#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

//

#include <Scene/Scene.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Renderer/Material/Builder.hpp>

//

#include <RHI/Swapchain.hpp>
#include <RHI/Resource/Resource.hpp>
#include <RHI/Commands/List.hpp>

//

namespace ranges = std::ranges;

namespace Neon::RG
{
    using namespace Scene;
    using namespace Renderer;

    ScenePass::ScenePass(
        const GraphStorage&                   Storage,
        const Ptr<Asset::ShaderLibraryAsset>& ShaderLibrary,
        GameScene&                            Scene) :
        IRenderPass(PassQueueType::Direct),
        m_Scene(Scene)
    {
        m_SpriteQuery = m_Scene->query_builder<
                                   Component::Transform,
                                   Component::Sprite>()
                            .build();

        //

        RenderMaterialBuilder Builder;

        Builder.ShaderLibrary(ShaderLibrary)
            .VertexShader(Asset::ShaderModuleId(0))
            .PixelShader(Asset::ShaderModuleId(0))
            .Rasterizer(MaterialStates::Rasterizer::CullNone)
            .DepthStencil(MaterialStates::DepthStencil::None)
            .RenderTarget(0, "Base Color", RHI::EResourceFormat::R8G8B8A8_UNorm)
            .Topology(RHI::PrimitiveTopology::TriangleList);

        {
            auto& VarMap = Builder.VarMap();

            VarMap.Add("Texture", { 0, 0 }, MaterialVarType::Resource)
                .Visibility(RHI::ShaderVisibility::Pixel)
                .Flags(EMaterialVarFlags::Shared, true);

            for (uint32_t i : ranges::iota_view(0u, uint32_t(MaterialStates::Sampler::_Last)))
            {
                auto Name = StringUtils::Format("StaticSampler_{}", i);
                VarMap.AddStaticSampler(Name, { i, 0 }, RHI::ShaderVisibility::Pixel, MaterialStates::Sampler(i));
            }
        }

        auto Mat = IMaterial::Create(Builder);

        m_SpriteBatch.reset(
            NEON_NEW SpriteBatch(
                std::move(Mat)));
    }

    void ScenePass::ResolveShaders(
        ShaderResolver& Resolver)
    {
    }

    void ScenePass::ResolveRootSignature(
        RootSignatureResolver& Resolver)
    {
    }

    void ScenePass::ResolvePipelineStates(
        PipelineStateResolver& Resolver)
    {
    }

    void ScenePass::ResolveResources(
        ResourceResolver& Resolver)
    {
        Resolver.WriteResource(
            RG::ResourceViewId(STR("FinalImage"), STR("ScenePass")),
            RHI::RTVDesc{
                .View      = RHI::RTVDesc::Texture2D{},
                .ClearType = RHI::ERTClearType::Color,
                .Format    = RHI::ISwapchain::Get()->GetFormat(),
            });

        //
    }

    void ScenePass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        if (m_SpriteQuery.is_true())
        {
            m_SpriteBatch->Begin(RenderCommandList);
            m_SpriteQuery.each(
                [this](const Component::Transform& Transform, const Component::Sprite& Sprite)
                {
                    m_SpriteBatch->Draw(
                        Renderer::SpriteBatch::QuadCommand{
                            .Position = Transform.World.GetPosition(),
                            .Size     = Sprite.Size,
                            .Color    = Sprite.ModulationColor }

                    );
                });
            m_SpriteBatch->End();
        }
    }
} // namespace Neon::RG