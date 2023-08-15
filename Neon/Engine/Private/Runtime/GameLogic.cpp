#include <EnginePCH.hpp>
#include <Physics/World.hpp>

#include <Runtime/GameLogic.hpp>
#include <Runtime/GameEngine.hpp>

#include <RHI/Swapchain.hpp>
#include <RenderGraph/RG.hpp>

#include <Scene/Component/Physics.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Exports/Export.hpp>

#include <Log/Logger.hpp>

namespace Neon::Runtime
{
    static std::mutex s_FlecsWorldMutex;

    using namespace Scene;

    GameLogic::GameLogic() :
        m_PhysicsWorld(std::make_unique<Physics::World>())
    {
        // Create entity world.
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_EntityWorld = std::make_unique<flecs::world>();
        }

        // Register components & relations.
        Exports::RegisterComponents(*m_EntityWorld);
        Exports::RegisterRelations(*m_EntityWorld);

        // Create physics update system.
        m_EntityWorld->system("PhysicsUpdate")
            .no_readonly()
            .kind(flecs::PreUpdate)
            .iter(
                [this](flecs::iter Iter)
                {
                    double DeltaTime = Runtime::DefaultGameEngine::Get()->GetDeltaTime();
                    m_PhysicsWorld->Update(Iter.world(), DeltaTime);
                });

        // Create physics collision add/remove system.
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

        // Create physics camera render system.
        m_CameraQuery =
            m_EntityWorld
                ->query_builder<Component::Camera>()
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

    GameLogic::~GameLogic()
    {
        if (m_EntityWorld)
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_EntityWorld.reset();
        }
    }

    GameLogic* GameLogic::Get()
    {
        return Runtime::DefaultGameEngine::Get()->GetLogic();
    }

    //

    void GameLogic::Render()
    {
        auto MainCamera = m_EntityWorld->target<Component::MainCamera>();
        m_CameraQuery.each(
            [MainCamera](flecs::entity      Entity,
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
                    RenderGraph->Run(MainCamera == Entity);
                }
            });
    }

    void GameLogic::Update()
    {
        float DeltaTime = float(Runtime::DefaultGameEngine::Get()->GetDeltaTime());
        m_EntityWorld->progress(DeltaTime);
    }

    //

    flecs::entity GameLogic::CreateEntity(
        const char* Name)
    {
        return m_EntityWorld->entity(Name);
    }

    flecs::entity GameLogic::CreateEntityInRoot(
        const char* Name)
    {
        return CreateEntity(Name);
        // auto RootEntity = GetRootEntity();
        // NEON_ASSERT(RootEntity, "Root entity not found");
        // return CreateEntity(Name).child_of(RootEntity);
    }

    flecs::entity GameLogic::CreateRootEntity(
        const char* Name)
    {
        auto Entity = CreateEntity(Name);
        m_EntityWorld->add<Component::Root>(Entity);
        return Entity;
    }

    flecs::entity GameLogic::GetRootEntity() const
    {
        return m_EntityWorld->target<Component::Root>();
    }

    void GameLogic::SetRootEntity(
        const flecs::entity& Entity)
    {
        m_EntityWorld->add<Component::Root>(Entity);
    }
} // namespace Neon::Runtime