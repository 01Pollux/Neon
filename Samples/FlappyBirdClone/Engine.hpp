#pragma once

#include <Runtime/GameEngine.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

#include <Asset/Types/Texture.hpp>

class FlappyBirdClone : public Neon::Runtime::DefaultGameEngine
{
public:
    void Initialize(
        Neon::Config::EngineConfig Config) override
    {
        DefaultGameEngine::Initialize(std::move(Config));
        m_Runtime = RegisterInterface<Neon::Runtime::IEngineRuntime, Neon::Runtime::EngineWorldRuntime>();

        // Preload the sprite
        PreloadSprite();
    }

private:
    /// <summary>
    /// Preload the sprite texture
    /// </summary>
    void PreloadSprite();

private:
    Neon::Ptr<Neon::Runtime::EngineWorldRuntime> m_Runtime;

    Neon::Asset::AssetTaskPtr<Neon::Asset::TextureAsset> m_Sprite;
};
