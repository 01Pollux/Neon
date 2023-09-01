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
        void ApplyToRoot(
            MergeType Type) const;

        /// <summary>
        /// Apply the scene to the world.
        /// </summary>
        void ApplyToScene(
            MergeType     Type,
            RuntimeScene& Other) const;

        /// <summary>
        /// Get the tag of the scene.
        /// </summary>
        [[nodiscard]] EntityHandle GetTag() const;

    public:
        /// <summary>
        /// Add an entity to the scene.
        /// </summary>
        EntityHandle CreateEntity(
            const char* Name) const;

        /// <summary>
        /// Add an entity to the scene.
        /// If the entity belongs to another scene, it will be transferred to this scene.
        /// </summary>
        void AddEntity(
            EntityHandle EntHandle) const;

        /// <summary>
        /// Clone an entity from another scene.
        /// </summary>
        void CloneEntity(
            EntityHandle EntHandle) const;

    private:
        /// <summary>
        /// Release the scene.
        /// </summary>
        void Release();

    private:
        EntityHandle m_SceneTag{};
    };
} // namespace Neon::Scene