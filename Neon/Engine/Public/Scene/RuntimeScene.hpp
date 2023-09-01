#pragma once

#include <Scene/EntityWorld.hpp>

namespace Neon::Scene
{
    class RuntimeScene
    {
    public:
        enum class MergeType : uint8_t
        {
            Merge,
            Replace
        };

        RuntimeScene();
        NEON_CLASS_NO_COPY(RuntimeScene);
        RuntimeScene(RuntimeScene&&) = default;
        RuntimeScene& operator=(RuntimeScene&& Other) noexcept;
        ~RuntimeScene();

    public:
        /// <summary>
        /// Apply the scene to the world.
        /// </summary>
        void ApplyTo(
            MergeType Type) const;

    public:
        /// <summary>
        /// Add an entity to the scene.
        /// </summary>
        EntityHandle CreateEntity(
            const char* Name);

        /// <summary>
        /// Add an entity to the scene.
        /// If the entity belongs to another scene, it will be transferred to this scene.
        /// </summary>
        void AddEntity(
            EntityHandle EntHandle);

        /// <summary>
        /// Clone an entity from another scene.
        /// </summary>
        void CloneEntity(
            EntityHandle EntHandle);

    private:
        /// <summary>
        /// Release the scene.
        /// </summary>
        void Release();

    private:
        EntityHandle m_SceneTag;
    };
} // namespace Neon::Scene