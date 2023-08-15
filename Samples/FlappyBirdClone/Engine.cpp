#include "Engine.hpp"

using namespace Neon;

void FlappyBirdClone::Initialize(
    Config::EngineConfig Config)
{
    GameEngine::Initialize(std::move(Config));

    PreloadSprite();

    PreloadMaterials();

    LoadScene();
}

Ptr<Neon::Renderer::IMaterial> FlappyBirdClone::GetMaterial(
    const StringU8& Name)
{
    auto Iter = m_Materials.find(Name);
    return Iter == m_Materials.end() ? nullptr : Iter->second;
}
