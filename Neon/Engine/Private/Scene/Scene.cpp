#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Exports/Export.hpp>

//

#include <Renderer/RG/RG.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>

//

#include <RHI/Resource/State.hpp>
#include <RHI/Swapchain.hpp>

namespace Neon::Scene
{
    static std::mutex s_FlecsWorldMutex;

    GameScene::GameScene()
    {
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_World = std::make_unique<flecs::world>();
        }

        Exports::RegisterComponents(*m_World);

#if NEON_DEBUG
        m_World->set<flecs::Rest>({});
        m_World->import <flecs::monitor>();
#endif

        m_CameraQuery =
            m_World->query_builder<
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
        if (m_World)
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_World.reset();
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
        auto MainCamera = m_World->get<Component::MainCamera>();
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
} // namespace Neon::Scene