#pragma once

#include <Runtime/GameEngine.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

#include <Renderer/Material/Material.hpp>
#include <Asset/Types/Texture.hpp>

class FlappyBirdClone : public Neon::Runtime::DefaultGameEngine
{
    using EngineWorldRuntimePtr = Neon::Ptr<Neon::Runtime::EngineWorldRuntime>;
    using MaterialMap           = std::unordered_map<Neon::String, Ptr<Neon::Renderer::IMaterial>>;
    using SpriteAssetPtr        = Neon::Asset::AssetTaskPtr<Neon::Asset::TextureAsset>;

public:
    void Initialize(
        Neon::Config::EngineConfig Config) override;

private:
    /// <summary>
    /// Preload the sprite texture
    /// </summary>
    void PreloadSprite();

    /// <summary>
    /// Preload the materials used in the scene.
    /// </summary>
    void PreloadMaterials();

    /// <summary>
    /// Load the scene
    /// </summary>
    void LoadScene();

private:
    EngineWorldRuntimePtr m_Runtime;
    MaterialMap           m_Materials;
    SpriteAssetPtr        m_Sprite;
};
