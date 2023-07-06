#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

//

#include <Scene/Scene.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Renderer/Material/Builder.hpp>

//

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
            .PixelShader(Asset::ShaderModuleId(0));

        {
            auto& VarMap = Builder.VarMap();

            VarMap.Add("Texture", { 0, 0 }, MaterialVarType::Resource)
                .Visibility(RHI::ShaderVisibility::Pixel)
                .Flags(EMaterialVarFlags::Shared, true);

#if 1

            VarMap.Add("T1S0", { 1, 0 }, MaterialVarType::Resource)
                .Visibility(RHI::ShaderVisibility::Pixel)
                .Flags(EMaterialVarFlags::Shared, true);

            VarMap.Add("TInfS0", { 2, 0 }, MaterialVarType::Resource)
                .Visibility(RHI::ShaderVisibility::Pixel);

            VarMap.Add("TInfS1", { 0, 1 }, MaterialVarType::Resource)
                .Visibility(RHI::ShaderVisibility::Pixel);

            VarMap.Add("T0InfS4", { 0, 3 }, MaterialVarType::Resource)
                .ArraySize(10)
                .Visibility(RHI::ShaderVisibility::Pixel);

            VarMap.Add("T0InfS4Vtx", { 0, 4 }, MaterialVarType::Resource)
                .ArraySize(10)
                .Visibility(RHI::ShaderVisibility::Vertex);

            VarMap.Add("T0InfS5All", { 0, 5 }, MaterialVarType::Resource)
                .ArraySize(10)
                .Visibility(RHI::ShaderVisibility::All);

            VarMap.Add("T0InfS6All", { 0, 6 }, MaterialVarType::Resource)
                .Visibility(RHI::ShaderVisibility::All);

#endif

            for (uint32_t i : ranges::iota_view(0u, uint32_t(MaterialCommon::Sampler::_Last)))
            {
                auto Name = StringUtils::Format("StaticSampler_{}", i);
                VarMap.AddStaticSampler(Name, { i, 0 }, RHI::ShaderVisibility::Pixel, MaterialCommon::Sampler(i));
            }
        }

        auto Mat = std::make_shared<Material>(Storage.GetSwapchain(), Builder);

        m_SpriteBatch.reset(
            NEON_NEW SpriteBatch(
                std::move(Mat),
                Storage.GetSwapchain()));
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
                .Format    = Resolver.GetSwapchainFormat(),
            });

        //
    }

    void ScenePass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        return;
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