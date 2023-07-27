#pragma once

#include <Runtime/GameEngine.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

#include <Renderer/Material/Material.hpp>
#include <RHI/Resource/Resource.hpp>
#include <Asset/Types/Texture.hpp>

#include <unordered_map>

class FlappyBirdClone : public Neon::Runtime::DefaultGameEngine
{
    using EngineWorldRuntimePtr = Neon::Ptr<Neon::Runtime::EngineWorldRuntime>;
    using MaterialMap           = std::unordered_map<std::string, Neon::Ptr<Neon::Renderer::IMaterial>>;
    using TextureAssetPtr       = Neon::Ptr<Neon::RHI::ITexture>;

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
    /// <summary>
    /// Get the material by name
    /// </summary>
    [[nodiscard]] Neon::Ptr<Neon::Renderer::IMaterial> GetMaterial(
        const Neon::StringU8& Name);

private:
    EngineWorldRuntimePtr m_Runtime;
    MaterialMap           m_Materials;
    TextureAssetPtr       m_Sprite;
};