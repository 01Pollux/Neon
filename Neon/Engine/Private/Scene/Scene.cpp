#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Exports/Export.hpp>

#include <Scene/Component/Sprite.hpp>

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
        m_World->component<Component::Sprite>().is_a<Component::CanvasItem>();

        Component::Sprite Sprite;
        Sprite.ModulationColor = Colors::BlueViolet;
        Entity.set<Component::Sprite>(Sprite);
    }
} // namespace Neon::Scene