#include "Engine.hpp"
#include <Asset/Manager.hpp>

#include <Scene/Component/Sprite.hpp>

using namespace Neon;

void FlappyBirdClone::Initialize(
    Config::EngineConfig Config)
{
    DefaultGameEngine::Initialize(std::move(Config));
    m_Runtime = RegisterInterface<Runtime::IEngineRuntime, Runtime::EngineWorldRuntime>();

    PreloadSprite();

    PreloadMaterials();

    LoadScene();
}
