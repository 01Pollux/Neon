#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>

namespace Neon::Scene
{
    static std::mutex s_FlecsWorldMutex;

    GameScene::GameScene()
    {
        std::scoped_lock Lock(s_FlecsWorldMutex);
        m_World = std::make_unique<flecs::world>();
        m_World->add<flecs::Rest>();
    }

    GameScene::~GameScene()
    {
        if (m_World)
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_World.reset();
        }
    }

    void GameScene::Test()
    {
        Actor Entity = m_World->entity("Spike");
        Entity.set<Sprite>(

        );
    }
} // namespace Neon::Scene