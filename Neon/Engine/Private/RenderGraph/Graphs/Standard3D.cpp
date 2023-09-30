#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard.hpp>

#include <RenderGraph/Passes/DepthPrepass.hpp>
#include <RenderGraph/Passes/ShadingPass.hpp>
#include <RenderGraph/Passes/CopyToTexture.hpp>

//

namespace Neon::RG
{
    void CreateStandard3DRenderGraph(
        Scene::Component::Camera& CameraComponent,
        const flecs::entity&      Camera)
    {
        auto Graph   = CameraComponent.NewRenderGraph(Camera);
        auto Builder = Graph->Reset();

        Builder.AddPass<DepthPrepass>();
        Builder.AddPass<ShadingPass>();
        Builder.Build();
    }
} // namespace Neon::RG