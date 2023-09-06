#include <EnginePCH.hpp>
#include <Physics/World.hpp>

#include <Runtime/GameLogic.hpp>
#include <Runtime/GameEngine.hpp>

#include <RHI/Swapchain.hpp>
#include <RenderGraph/RG.hpp>

#include <Scene/Component/Physics.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Transform.hpp>

#include <Scene/Component/Script.hpp>
#include <Script/Engine.hpp>

#include <Log/Logger.hpp>

#include <Scene/RuntimeScene.hpp>

namespace Neon::Runtime
{
    static std::mutex s_FlecsWorldMutex;

    using namespace Scene;

    GameLogic::GameLogic() :
        m_PhysicsWorld(std::make_unique<Physics::World>())
    {
        auto World = EntityWorld::Get();

        // Create physics update system.
        World
            .system("PhysicsUpdate")
            .no_readonly()
            .kind(flecs::PreUpdate)
            .iter(
                [this](flecs::iter Iter)
                {
                    double DeltaTime = Runtime::GameEngine::Get()->GetDeltaTime();
                    m_PhysicsWorld->Update(Iter.world(), DeltaTime);
                });

        // Create physics collision add/remove system.
        World
            .observer<Component::CollisionObject>("Physics(Add/Remove)")
            .with<Component::CollisionShape>()
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

        // Create script add/remove system.
        World
            .observer<Component::ScriptInstance>("Script(Add/Remove)")
            .event(flecs::OnRemove)
            .event(flecs::OnSet)
            .each(
                [this](flecs::iter& Iter, size_t Idx, Component::ScriptInstance& Instance)
                {
                    if (Iter.event() == flecs::OnRemove)
                    {
                        if (Instance.Handle)
                        {
                            Instance.Handle.Free();
                            Instance.Handle = {};
                        }
                    }
                    else if (Iter.event() == flecs::OnSet)
                    {
                        Instance.Handle = Scripting::CreateNeonScriptObject(
                            Instance.AssemblyName.c_str(),
                            Instance.ClassName.c_str(),
                            Iter.entity(Idx));
                    }
                });

        // Create camera position update system.
        World
            .system<Component::Camera, Component::Transform>("CameraPositionUpdate")
            .kind(flecs::PreUpdate)
            .term<Component::Camera>()
            .inout()
            .term<Component::Transform>()
            .in()
            .each(
                [](Component::Camera&          Camera,
                   const Component::Transform& Transform)
                {
                    Camera.SetCurrentPosition(Transform.World.GetPosition());
                    if (auto RenderGraph = Camera.GetRenderGraph())
                    {
                        RenderGraph->Update(
                            Camera,
                            Transform);
                    }
                });

        // Create camera render system.
        m_CameraQuery =
            World
                .query_builder<Component::Camera>()
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

        // Create an observer for entities that are part of scene
        World.observer("SceneObserver")
            .term<Component::WorldSceneRoot>(flecs::Wildcard)
            .singleton()
            .event(flecs::OnAdd)
            .event(flecs::OnRemove)
            .iter(
                [this](flecs::iter& Iter)
                {
                    auto World  = Iter.world();
                    auto Target = World.target<Component::WorldSceneRoot>();
                    auto Filter = World.filter_builder()
                                      .with<Component::SceneEntity>(Target)
                                      .build();

                    if (Iter.event() == flecs::OnAdd)
                    {
                        Filter.each([](flecs::entity Entity)
                                    { Entity.add<Component::ActiveSceneEntity>(); });
                    }
                    else
                    {
                        Filter.each([](flecs::entity Entity)
                                    { Entity.remove<Component::ActiveSceneEntity>(); });
                    }
                });
    }

    GameLogic::~GameLogic() = default;

    GameLogic* GameLogic::Get()
    {
        return Runtime::GameEngine::Get()->GetLogic();
    }

    //

    void GameLogic::Render()
    {
        m_CameraQuery.each(
            [](flecs::entity      Entity,
               Component::Camera& Camera)
            {
                auto& Size = RHI::ISwapchain::Get()->GetSize();

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
                    RenderGraph->Draw();
                }
            });
    }

    void GameLogic::Update()
    {
        float DeltaTime = float(Runtime::GameEngine::Get()->GetDeltaTime());
        EntityWorld::Get().progress(DeltaTime);
    }
} // namespace Neon::Runtime