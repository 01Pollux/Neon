#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Exports/Export.hpp>

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
    }

    GameScene::~GameScene()
    {
        if (m_World)
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_World.reset();
        }
    }
} // namespace Neon::Scene