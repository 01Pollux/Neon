#include "Engine.hpp"

void FlappyBirdClone::LoadScene()
{
    auto& Scene = m_Runtime->GetScene();

    auto PlayerSprite    = Scene.CreateEntity(Scene::EntityType::Sprite, "PlayerSprite");
    auto SpriteComponent = PlayerSprite.get_mut<Scene::Component::Sprite>();
}
