#pragma once

#include <Math/Matrix.hpp>
#include <Math/Vector.hpp>
#include <Math/Common.hpp>

#include <RHI/Resource/Resource.hpp>
#include <Allocator/Buddy.hpp>
#include <vector>

namespace Neon::Scene
{
    namespace Component
    {
        struct Light;
    } // namespace Component

    class GPULightManager
    {
        enum class LightFlags : uint32_t
        {
            Directional = 1 << 0,
            Point       = 1 << 1,
            Spot        = 1 << 2,
            TypeMask    = Directional | Point | Spot,

            Enabled = 1 << 3
        };

        struct InstanceData
        {
            Vector4  Color{};
            Vector3  PositionVS{};
            float    Range = 0.f; // 0 for directional light, Point light + spot light
            Vector3  Direction{};
            uint32_t Flags = 0;           // LightFlags::* flags
            Vector3  Attenuation_Angle{}; // 0 for directional light, (attenuation, 0, 0) for point light, (attenuation, angle, attenuation) for spot light
        };

        using InstanceIdList          = std::vector<uint32_t>;
        using InstanceIdPipelineGroup = std::unordered_map<RHI::IPipelineState*, InstanceIdList>;

        using InstanceIdLightMap = std::unordered_map<uint32_t, const Component::Light*>;

    public:
        static constexpr size_t   MaxLightsInScene   = 1024; // MAX_LIGHTS
        static constexpr uint32_t InvalidInstanceId  = std::numeric_limits<uint32_t>::max();
        static constexpr size_t   SizeOfInstanceData = Math::AlignUp(sizeof(InstanceData), 16);

    public:
        GPULightManager();

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
        [[nodiscard]] auto& GetLightInstanceIds() const noexcept
        {
            return m_LightInstanceIds;
        }

        [[nodiscard]] auto& GetMeshes() const noexcept
        {
            return m_Lights;
        }

    private:
        Ptr<RHI::IGpuResource>    m_LightsBuffer;
        Allocator::BuddyAllocator m_LightsInScene;
        InstanceData*             m_LightsBufferPtr;

        InstanceIdLightMap      m_Lights;
        InstanceIdPipelineGroup m_LightInstanceIds;
    };
} // namespace Neon::Scene