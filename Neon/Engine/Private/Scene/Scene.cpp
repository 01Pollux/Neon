#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Exports/Export.hpp>
#include <Physics/World.hpp>

//

#include <Renderer/RG/RG.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Physics.hpp>
#include <Scene/Component/Camera.hpp>

//

#include <RHI/Resource/State.hpp>
#include <RHI/Swapchain.hpp>

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
                    m_PhysicsWorld->Update(m_GameTimer.GetDeltaTime());
                });

        m_EntityWorld->observer<Component::CollisionObject>("PhysicsAdd")
            .with<Component::CollisionShape>()
            .event(flecs::OnAdd)
            .event(flecs::OnRemove)
            .event(flecs::OnSet)
            .each(
                [this](flecs::iter& Iter, size_t, Component::CollisionObject& Object)
                {
                    if (Iter.event() == flecs::OnRemove)
                    {
                        m_PhysicsWorld->RemovePhysicsObject(Object.BulletObject);
                    }
                    else if (Iter.event() == flecs::OnSet)
                    {
                        m_PhysicsWorld->RemovePhysicsObject(Object.BulletObject);
                        m_PhysicsWorld->AddPhysicsObject(Object.BulletObject, Object.Group, Object.Mask);
                    }
                });
        //

        m_CameraQuery =
            m_EntityWorld->query_builder<
                             Component::Transform,
                             Component::Camera>()
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
        m_CameraQuery.each(
            [](Actor                 Entity,
               Component::Transform& Transform,
               Component::Camera&    Camera)
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

                if (Camera.RenderGraph)
                {
                    Camera.RenderGraph->Run();
                }
            });

        // Rendering to back buffer for main camera
        auto MainCamera = m_EntityWorld->get<Component::MainCamera>();
        if (MainCamera)
        {
            auto& RenderGraph = MainCamera->Target.get<Component::Camera>()->RenderGraph;
            auto& Storage     = RenderGraph->GetStorage();

            auto OutputImage  = Storage.GetResource(RG::ResourceId(STR("OutputImage"))).AsTexture();
            auto Backbuffer   = RHI::ISwapchain::Get()->GetBackBuffer();
            auto StateManager = RHI::IResourceStateManager::Get();

            //

            // Transition the backbuffer to a copy destination.
            StateManager->TransitionResource(
                Backbuffer,
                RHI::MResourceState::FromEnum(RHI::EResourceState::CopyDest));

            // Transition the output image to a copy source.
            StateManager->TransitionResource(
                OutputImage.get(),
                RHI::MResourceState::FromEnum(RHI::EResourceState::CopySource));

            // Prepare the command list.
            auto CommandContext = StateManager->FlushBarriers();

            CommandContext[0]->CopyResources(Backbuffer, OutputImage.get());

            StateManager->TransitionResource(
                Backbuffer,
                RHI::MResourceState_Present);

            StateManager->FlushBarriers(CommandContext[0]);
        }
    }

    void GameScene::Update()
    {
        if (m_GameTimer.Tick())
        {
            m_EntityWorld->progress(float(m_GameTimer.GetDeltaTime()));
        }
    }

    //

    double GameScene::GetGameTime() const
    {
        return m_GameTimer.GetGameTime();
    }

    double GameScene::GetEngineTime() const
    {
        return m_GameTimer.GetEngineTime();
    }

    double GameScene::GetDeltaTime() const
    {
        return m_GameTimer.GetDeltaTime();
    }

    float GameScene::GetTimeScale() const
    {
        return m_GameTimer.GetTimeScale();
    }

    void GameScene::SetTimeScale(
        float TimeScale)
    {
        m_GameTimer.SetTimeScale(TimeScale);
    }
} // namespace Neon::Scene