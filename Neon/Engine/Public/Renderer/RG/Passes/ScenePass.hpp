#pragma once

#include <Renderer/RG/Pass.hpp>
#include <Renderer/RG/Graph.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Scene.hpp>

namespace Neon::RG
{
    class ScenePass : public IRenderPass
    {
    public:
        ScenePass(
            const GraphStorage& Storage,
            Scene::GameScene&   Scene,
            Scene::Actor        Camera);

        void ResolveShaders(
            ShaderResolver& Resolver) override;

        void ResolveRootSignature(
            RootSignatureResolver& Resolver) override;

        void ResolvePipelineStates(
            PipelineStateResolver& Resolver) override;

        void ResolveResources(
            ResourceResolver& Resolver) override;

        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList) override;

    private:
        /// <summary>
        /// Update camera buffer.
        /// </summary>
        void UpdateCameraBuffer();

    private:
        UPtr<Renderer::SpriteBatch> m_SpriteBatch;
        Ptr<RHI::IUploadBuffer>     m_CameraBuffer;

        Scene::GameScene& m_Scene;
        Scene::Actor      m_Camera;

        flecs::query<
            Scene::Component::Transform,
            Scene::Component::Sprite>
            m_SpriteQuery;
    };
} // namespace Neon::RG