#pragma once

#include <Scene/GPU/PagedInstance.hpp>

#include <Math/Matrix.hpp>
#include <Math/Vector.hpp>
#include <Math/Common.hpp>

#include <RHI/Resource/Resource.hpp>
#include <Allocator/Buddy.hpp>
#include <vector>

namespace Neon::Scene::Component
{
    struct Transform;
    struct MeshInstance;
} // namespace Neon::Scene::Component

namespace Neon::Scene
{
    class GPUTransformManager
    {
    public:
        struct RenderableHandle
        {
            constexpr bool Valid() const noexcept
            {
                return InstanceId != std::numeric_limits<uint32_t>::max();
            }

            /// <summary>
            /// Instance ID of the renderable in GPUTransformManager.
            /// </summary>
            uint32_t InstanceId = std::numeric_limits<uint32_t>::max();
        };

        struct InstanceData
        {
            Matrix4x4 World;
        };

        using PagedInstaceData                        = GPUPagedInstance<InstanceData>;
        static constexpr size_t   SizeOfInstanceData  = PagedInstaceData::SizeOfInstanceData;
        static constexpr uint32_t AlignOfInstanceData = PagedInstaceData::SizeOfInstanceData;

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

    private:
        GPUPagedInstance<InstanceData> m_PagesInstances;
    };
} // namespace Neon::Scene