#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Impots/Import.hpp>

#include <Scene/Component/Sprite.hpp>

namespace Neon::Scene
{
    static std::mutex s_FlecsWorldMutex;

    GameScene::GameScene()
    {
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_World = std::make_unique<flecs::world>();
            Imports::Import(*m_World);
        }
        m_World->set<flecs::Rest>({});
        m_World->import <flecs::monitor>();
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
        m_World->component<Component::Sprite2>().is_a<Component::CanvasItem>();

        Component::Sprite  Sprite;
        Component::Sprite2 Sprite2;
        Sprite.ModulationColor = Colors::BlueViolet;
        Entity.set<Component::Sprite>(Sprite);
        Entity.set<Component::Sprite2>(Sprite2);
    }
} // namespace Neon::Scene