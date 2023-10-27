#pragma once

#include <RenderGraph/Common.hpp>
#include <Math/Common.hpp>

namespace Neon
{
    namespace RHI
    {
        class IMaterial;
    } // namespace RHI

    namespace Scene::Component
    {
        struct Camera;
        struct Transform;
        struct MeshInstance;
        struct CSGShape;
    } // namespace Scene::Component
} // namespace Neon

namespace Neon::RG
{
    class GraphStorage;

    class SceneContext
    {
    public:
        enum class RenderType : uint8_t
        {
            DepthPrepass,
            RenderPass
        };

    private:
        using MeshQuery = flecs::query<
            const Scene::Component::Transform,
            const Scene::Component::MeshInstance>;

        using CSGRule = flecs::rule<
            const Scene::Component::Transform,
            const Scene::Component::CSGShape>;

    private:
        struct EntityRenderInfo
        {
            Ptr<RHI::IMaterial> Material;

            RHI::GpuResourceHandle VertexBuffer;

            uint32_t VertexCount;
            uint32_t IndexCount;

            RHI::GpuResourceHandle IndexBuffer;

            uint32_t VertexOffset;
            uint32_t IndexOffset;

            uint32_t               InstanceId;
            RHI::PrimitiveTopology Topology;
        };

    private:
        enum class EntityType : uint8_t
        {
            Mesh,
            CSG
        };

        struct EntityInfo
        {
            flecs::entity_t Id;
            float           Dist;
            EntityType      Type;

        public:
            /// <summary>
            /// Check to see if the entity can be rendered (used for compute materials)
            /// </summary>
            [[nodiscard]] bool CanRender(
                RenderType PassType,
                uint32_t   PassIndex) const;

            /// <summary>
            /// Get data needed for rendering
            /// </summary>
            [[nodiscard]] EntityRenderInfo GetRenderInfo() const;

            auto operator<=>(const EntityInfo& Other) const noexcept
            {
                return Dist <=> Other.Dist;
            }
        };

        using EntityList      = std::set<EntityInfo>;
        using EntityListGroup = std::map<RHI::IPipelineState*, EntityList>;

    public:
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
        CSGRule   m_CSGRule;

        EntityListGroup m_EntityLists;
    };
} // namespace Neon::RG