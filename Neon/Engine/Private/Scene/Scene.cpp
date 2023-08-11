#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>
#include <Physics/World.hpp>
#include <Runtime/GameEngine.hpp>
#include <Scene/Exports/Export.hpp>

//

#include <RHI/Swapchain.hpp>
#include <RenderGraph/RG.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Physics.hpp>
#include <Scene/Component/Camera.hpp>

namespace Neon::Scene
{
    static std::mutex s_FlecsWorldMutex;

    GameScene::GameScene() :
        m_PhysicsWorld(std::make_unique<Physics::World>())
    {
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_EntityWorld = std::make_unique<flecs::world>();
        }

        Exports::RegisterComponents(*m_EntityWorld);

#if NEON_DEBUG
        m_EntityWorld->set<flecs::Rest>({});
        m_EntityWorld->import <flecs::monitor>();
#endif

        m_EntityWorld->system("PhysicsUpdate")
            .kind(flecs::PreUpdate)
            .iter(
                [this](flecs::iter)
                {
                    double DeltaTime = Runtime::DefaultGameEngine::Get()->GetDeltaTime();
                    m_PhysicsWorld->Update(DeltaTime);
                });

        m_EntityWorld->observer<Component::CollisionObject>("Physics(Add/Remove)")
            .with<Component::CollisionShape>()
            .event(flecs::OnAdd)
            .event(flecs::OnRemove)
            .event(flecs::OnSet)
            .each(
                [this](flecs::iter& Iter, size_t, Component::CollisionObject& Object)
                {
                    if (Iter.event() == flecs::OnRemove)
                    {
                        m_PhysicsWorld->RemovePhysicsObject(Object.BulletObject.get());
                    }
                    else if (Iter.event() == flecs::OnSet)
                    {
                        m_PhysicsWorld->RemovePhysicsObject(Object.BulletObject.get());
                        m_PhysicsWorld->AddPhysicsObject(Object.BulletObject.get(), Object.Group, Object.Mask);
                    }
                });
        //

        m_CameraQuery =
            m_EntityWorld
                ->query_builder<Component::Transform, Component::Camera>()
                .term<Component::Transform>()
                .in()
                .term<Component::Camera>()
                .inout()
                .order_by(
                    +[](flecs::entity_t,
                        const Component::Camera* LhsCamera,
                        flecs::entity_t,
                        const Component::Camera* RhsCamera) -> int
                    {
                        return int(RhsCamera->RenderPriority - LhsCamera->RenderPriority);
                    })
                .build();
    }

    GameScene::~GameScene()
    {
        if (m_EntityWorld)
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_EntityWorld.reset();
        }
    }

    //

    void GameScene::Render()
    {
        auto MainCamera = m_EntityWorld->get<Component::MainCamera>();
        m_CameraQuery.each(
            [MainCamera](flecs::entity               Entity,
                         const Component::Transform& Transform,
                         Component::Camera&          Camera)
            {
                auto Size = RHI::ISwapchain::Get()->GetSize();

                if (Camera.Viewport.ClientWidth)
                {
                    Camera.Viewport.Width = float(Size.Width());
                }
                if (Camera.Viewport.ClientHeight)
                {
                    Camera.Viewport.Height = float(Size.Height());
                }

                if (auto RenderGraph = Camera.GetRenderGraph())
                {
                    RenderGraph->Run(MainCamera->Target == Entity);
                }
            });
    }

    void GameScene::Update()
    {
        float DeltaTime = float(Runtime::DefaultGameEngine::Get()->GetDeltaTime());
        m_EntityWorld->progress(DeltaTime);
    }
} // namespace Neon::Scene