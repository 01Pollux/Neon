#pragma once

#include <Scene/GPU/PagedInstance.hpp>

#include <Math/Matrix.hpp>
#include <Math/Vector.hpp>
#include <Math/Common.hpp>

#include <RHI/Resource/Resource.hpp>
#include <Allocator/Buddy.hpp>
#include <vector>

namespace Neon::Mdl
{
    class Mesh;
}

namespace Neon::Scene
{
    class GPUTransformManager
    {
        struct InstanceData
        {
            Matrix4x4 World;
        };

        using PagedInstaceData        = GPUPagedInstance<InstanceData>;
        using InstanceIdList          = PagedInstaceData::InstanceIdList;
        using InstanceIdPipelineGroup = std::unordered_map<RHI::IPipelineState*, InstanceIdList>;
        using InstanceIdMeshMap       = std::unordered_map<uint32_t, const Mdl::Mesh*>;

    public:
        GPUTransformManager();

    public:
        /// <summary>
        /// Create a new instance data, returns instance id and fill the instance data pointer if not null
        /// </summary>
        [[nodiscard]] uint32_t AddInstance(
            InstanceData** InstanceData = nullptr);

        /// <summary>
        /// Remove an instance data
        /// </summary>
        void RemoveInstance(
            uint32_t InstanceId);

        /// <summary>
        /// Get the instance data
        /// </summary>
        [[nodiscard]] InstanceData* GetInstanceData(
            uint32_t InstanceId);

        /// <summary>
        /// Get the instance data
        /// </summary>
        [[nodiscard]] const InstanceData* GetInstanceData(
            uint32_t InstanceId) const;

        /// <summary>
        /// Get the instance resource's handle
        /// </summary>
        [[nodiscard]] RHI::GpuResourceHandle GetInstanceHandle(
            uint32_t InstanceId) const;

    public:
        [[nodiscard]] auto& GetMeshInstanceIds() const noexcept
        {
            return m_MeshInstanceIds;
        }

        [[nodiscard]] auto& GetMeshes() const noexcept
        {
            return m_Meshes;
        }

    private:
        GPUPagedInstance<InstanceData> m_PagesInstances;
        InstanceIdMeshMap              m_Meshes;
        InstanceIdPipelineGroup        m_MeshInstanceIds;
    };
} // namespace Neon::Scene