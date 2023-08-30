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
        /// Get the root entity of the scene.
        /// </summary>
        [[nodiscard]] EntityHandle GetRoot() const;

        /// <summary>
        /// Apply the scene to the world.
        /// </summary>
        void Apply(
            MergeType Type);

        /// <summary>
        /// Apply the scene to the root.
        /// </summary>
        void Apply(
            MergeType    Type,
            EntityHandle DestRoot);

    private:
        EntityHandle m_RootEntity;
    };
} // namespace Neon::Scene