#pragma once

#include <Runtime/GameEngine.hpp>
#include <Scene/Scene.hpp>

#include <RHI/Resource/Resource.hpp>
#include <Asset/Types/Texture.hpp>

#include <Renderer/Material/Material.hpp>

#include <unordered_map>

class FlappyBirdClone : public Neon::Runtime::DefaultGameEngine
{
    using MaterialMap     = std::unordered_map<Neon::StringU8, Neon::Ptr<Neon::Renderer::IMaterial>>;
    using TextureAssetPtr = Neon::Ptr<Neon::RHI::ITexture>;

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

    /// <summary>
    /// Attach the inputs
    /// </summary>
    void AttachInputs();

    /// <summary>
    /// Called by pipeline on update
    /// </summary>
    void OnUpdate();

private:
    /// <summary>
    /// Get the material by name
    /// </summary>
    [[nodiscard]] Neon::Ptr<Neon::Renderer::IMaterial> GetMaterial(
        const Neon::StringU8& Name);

private:
    MaterialMap m_Materials;

    TextureAssetPtr m_Sprite;
    TextureAssetPtr m_HdrTriangle;

    Neon::Scene::Actor m_Player;

    /// <summary>
    /// Pointer to player's rigid body
    /// </summary>
    class btRigidBody* m_RigidBody = nullptr;

    /// <summary>
    /// Accumulated velocity of the player
    /// </summary>
    float m_VelocityAccum = 0.f;

    /// <summary>
    /// Engine power of the player
    /// </summary>
    float m_EnginePower = 4.0f;

    /// <summary>
    /// Temporary flag to check if the player is jumping until we have imgui
    /// </summary>
    bool m_IsJumping = false;
};
