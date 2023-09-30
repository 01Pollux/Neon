#pragma once

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
    class GPUScene
    {
        struct InstanceData
        {
            Matrix4x4 World;
        };

        static constexpr uint32_t InvalidInstanceId  = std::numeric_limits<uint32_t>::max();
        static constexpr size_t   SizeOfInstanceData = Math::AlignUp(sizeof(InstanceData), 16);
        static constexpr size_t   SizeOfPage         = std::numeric_limits<uint16_t>::max();
        static constexpr size_t   NumberOfPages      = std::numeric_limits<uint16_t>::max();

        using InstanceIdList          = std::vector<uint32_t>;
        using InstanceIdPipelineGroup = std::unordered_map<RHI::IPipelineState*, InstanceIdList>;

        using InstanceIdMeshMap = std::unordered_map<uint32_t, const Mdl::Mesh*>;

    public:
        using InstanceDataBuffer = Ptr<RHI::IGpuResource>;
        struct InstanceBufferPage
        {
            InstanceDataBuffer        Instances;
            Allocator::BuddyAllocator Allocator;
            InstanceData*             MappedInstances;

            InstanceBufferPage(
                size_t PageIndex);
        };

        using PagedInstanceBufferArray = std::vector<InstanceBufferPage>;

    public:
        GPUScene();

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
        PagedInstanceBufferArray m_PagesInstances;

        InstanceIdMeshMap       m_Meshes;
        InstanceIdPipelineGroup m_MeshInstanceIds;
    };
} // namespace Neon::Scene