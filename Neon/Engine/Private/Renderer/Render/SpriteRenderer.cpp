#include <EnginePCH.hpp>
#include <Renderer/Render/SpriteRenderer.hpp>
#include <Scene/EntityWorld.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::Renderer
{
    SpriteRenderer::SpriteRenderer(
        uint32_t MaxSpritePerBatch) :
        m_SpriteBatch(MaxSpritePerBatch),
        m_SpriteQuery(
            Scene::EntityWorld::Get()
                .query_builder<Component::Transform, Component::Sprite>()
                .with<Component::ActiveSceneEntity>()
                .without<Component::Sprite::CustomRenderer>()
                .term<Component::Transform>()
                .in()
                .term<Component::Sprite>()
                .in()
                .build())
    {
    }

    SpriteRenderer::~SpriteRenderer()
    {
        m_SpriteQuery.destruct();
    }

    void SpriteRenderer::Render(
        RHI::GpuResourceHandle CameraBuffer,
        RHI::ICommandList*     CommandList)
    {
        if (m_SpriteQuery.is_true())
        {
            CommandList->MarkEvent("Render Sprite");
            m_SpriteBatch.SetCameraBuffer(CameraBuffer);
            m_SpriteBatch.Begin(CommandList);
            m_SpriteQuery.iter(
                [this](flecs::iter&                Iter,
                       const Component::Transform* Transform,
                       const Component::Sprite*    Sprite)
                {
                    for (size_t i : Iter)
                    {
                        m_SpriteBatch.Draw(Transform[i], Sprite[i]);
                    }
                });
            m_SpriteBatch.End();
        }
    }
} // namespace Neon::Renderer