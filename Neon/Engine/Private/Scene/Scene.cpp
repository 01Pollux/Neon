#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Exports/Export.hpp>

//

#include <Renderer/RG/RG.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>

//

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
        Exports::RegisterRelations(*m_World);

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
        auto& CameraQuery = m_CameraQuery;

        m_CameraQuery.each(
            [&](Actor                 Entity,
                Component::Transform& Transform,
                Component::Camera&    Camera)
            {
                if (Camera.RenderGraph)
                {
                    Camera.RenderGraph->Run();
                }
            });
    }
} // namespace Neon::Scene