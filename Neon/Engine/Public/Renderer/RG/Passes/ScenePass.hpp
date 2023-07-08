#pragma once

#include <Renderer/RG/Pass.hpp>
#include <Renderer/RG/Graph.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Resource/Types/Shader.hpp>

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
            Scene::GameScene&   Scene);

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
        UPtr<Renderer::SpriteBatch> m_SpriteBatch;

        Scene::GameScene& m_Scene;

        flecs::query<
            Scene::Component::Transform,
            Scene::Component::Sprite>
            m_SpriteQuery;
    };
} // namespace Neon::RG