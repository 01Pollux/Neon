#pragma once

#include <RenderGraph/Common.hpp>
#include <Math/Common.hpp>

namespace Neon::Scene::Component
{
    struct Camera;
    struct Transform;
    struct MeshInstance;
} // namespace Neon::Scene::Component

namespace Neon::RG
{
    class GraphStorage;

    class SceneContext
    {
        using MeshQuery = flecs::query<
            const Scene::Component::Transform,
            const Scene::Component::MeshInstance>;

        struct EntityInfo
        {
            flecs::entity_t Id;
            float           Dist;

            auto operator<=>(const EntityInfo& Other) const noexcept
            {
                return Dist <=> Other.Dist;
            }
        };

        using EntityList      = std::set<EntityInfo>;
        using EntityListGroup = std::map<RHI::IPipelineState*, EntityList>;

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

    public:
        /// <summary>
        /// Update the scene context's entities
        /// </summary>
        void Update(
            const Scene::Component::Camera&    Camera,
            const Scene::Component::Transform& Transform);

        /// <summary>
        /// Render the scene depending on the type
        /// </summary>
        void Render(
            RHI::ICommandList*       CommandList,
            RenderType               Type,
            RHI::GpuDescriptorHandle OpaqueLightDataHandle,
            RHI::GpuDescriptorHandle TransparentLightDataHandle) const;

    public:
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

        MeshQuery m_MeshQuery;

        EntityListGroup m_EntityLists;
    };
} // namespace Neon::RG