#pragma once

#include <RenderGraph/Common.hpp>
#include <Scene/GPU/Scene.hpp>
#include <Math/Common.hpp>

namespace Neon::RG
{
    class GraphStorage;

    class SceneContext
    {
        // Transform
        using QueryOrdererC = int(const flecs::entity_t, const void*, flecs::entity_t, const void*);
        using QueryOrderer  = std::function<QueryOrdererC>;

        using MeshQuery = flecs::query<
            const Scene::GPUTransformManager::RenderableHandle,
            const Scene::Component::Transform,
            const Scene::Component::MeshInstance>;

    public:
        enum class RenderType : uint8_t
        {
            DepthPrepass,
            RenderPass
        };

        SceneContext(
            const GraphStorage& Storage);

        NEON_CLASS_NO_COPYMOVE(SceneContext);

        ~SceneContext();

        /// <summary>
        /// Render the scene depending on the type
        /// </summary>
        void Render(
            RHI::ICommandList*       CommandList,
            RenderType               Type,
            RHI::GpuDescriptorHandle OpaqueLightDataHandle,
            RHI::GpuDescriptorHandle TransparentLightDataHandle) const;

        /// <summary>
        /// Get the light's resource view
        /// </summary>
        [[nodiscard]] RHI::CpuDescriptorHandle GetLightsResourceView() const noexcept;

        /// <summary>
        /// Get the light's resource count
        /// </summary>
        [[nodiscard]] uint32_t GetLightsCount() const noexcept;

    private:
        const GraphStorage& m_Storage;

        QueryOrderer m_TransformOrderer;

        MeshQuery m_MeshQuery;
    };
} // namespace Neon::RG