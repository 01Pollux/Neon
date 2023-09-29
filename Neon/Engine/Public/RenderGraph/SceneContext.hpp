#pragma once

#include <RenderGraph/Common.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Renderable.hpp>
#include <Scene/Component/Mesh.hpp>

namespace Neon::RG
{
    class GraphStorage;

    class SceneContext
    {
        using InstanceIdList          = std::vector<uint32_t>;
        using InstanceIdPipelineGroup = std::unordered_map<RHI::IPipelineState*, InstanceIdList>;

        using InstanceIdMeshMap   = std::unordered_map<uint32_t, const Mdl::Mesh*>;
        using InstanceIdMeshQuery = flecs::query<
            const Scene::Component::Transform,
            const Scene::Component::MeshInstance,
            const Scene::Component::Renderable>;

    public:
        enum class RenderType : uint8_t
        {
            // Deferred,
            DepthPrepass,
            RenderPass
        };

        SceneContext(
            const GraphStorage& Storage);

        /// <summary>
        /// Update the scene context
        /// </summary>
        void Update();

        /// <summary>
        /// Dispatch the renderers depending on the type
        /// </summary>
        void Render(
            RHI::ICommandList* CommandList,
            RenderType         Type) const;

    private:
        /// <summary>
        /// Update the instance ids for the renderers from the query
        /// </summary>
        void UpdateInstances();

    private:
        const GraphStorage& m_Storage;

        InstanceIdMeshQuery     m_MeshQuery;
        InstanceIdMeshMap       m_Meshes;
        InstanceIdPipelineGroup m_MeshInstanceIds;
    };
} // namespace Neon::RG