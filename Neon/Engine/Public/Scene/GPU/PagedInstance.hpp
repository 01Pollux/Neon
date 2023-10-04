#pragma once

#include <Math/Matrix.hpp>
#include <Math/Vector.hpp>
#include <Math/Common.hpp>

#include <RHI/Resource/Resource.hpp>
#include <Allocator/Buddy.hpp>
#include <vector>

namespace Neon::Scene
{
    template<typename _Ty>
    class GPUPagedInstance
    {
    public:
        using InstanceData = _Ty;

        static constexpr uint32_t InvalidInstanceId  = std::numeric_limits<uint32_t>::max();
        static constexpr size_t   SizeOfInstanceData = Math::AlignUp(sizeof(InstanceData), 16);
        static constexpr size_t   SizeOfPage         = std::numeric_limits<uint16_t>::max();
        static constexpr size_t   NumberOfPages      = std::numeric_limits<uint16_t>::max();

        using InstanceIdList          = std::vector<uint32_t>;
        using InstanceIdPipelineGroup = std::unordered_map<RHI::IPipelineState*, InstanceIdList>;

        using InstanceDataBuffer = Ptr<RHI::IGpuResource>;
        struct InstanceBufferPage
        {
            InstanceDataBuffer        Instances;
            Allocator::BuddyAllocator Allocator;
            InstanceData*             MappedInstances;

            InstanceBufferPage(
                size_t PageIndex) :
                Instances(RHI::IGpuResource::Create(
                    RHI::ResourceDesc::Buffer(
                        SizeOfPage * SizeOfInstanceData,
                        {},
                        RHI::GraphicsBufferType::Upload),
                    {
#ifndef NEON_DIST
                        .Name = StringUtils::Format(STR("InstanceBufferPage_{}"), PageIndex).c_str(),
#endif
                        .InitialState = RHI::IGpuResource::DefaultUploadResourceState })),
                Allocator(SizeOfPage),
                MappedInstances(Instances->Map<InstanceData>())
            {
            }
        };

        using PagedInstanceBufferArray = std::vector<InstanceBufferPage>;

    public:
        GPUPagedInstance()
        {
            // Reserve at least one page.
            m_PagesInstances.emplace_back(0);
        }

    public:
        /// <summary>
        /// Create a new instance data, returns instance id and fill the instance data pointer if not null
        /// </summary>
        [[nodiscard]] uint32_t AddInstance(
            InstanceData** InstanceData = nullptr)
        {
            uint32_t InstanceId = InvalidInstanceId;

            for (uint32_t i = 0; i < uint32_t(m_PagesInstances.size()); i++)
            {
                auto& Page = m_PagesInstances[i];
                if (auto Instance = Page.Allocator.Allocate(1))
                {
                    InstanceId = uint32_t(i << 16) | uint32_t(Instance.Offset);
                    if (InstanceData)
                    {
                        *InstanceData = Page.MappedInstances + Instance.Offset;
                    }
                }
            }

            if (InstanceId == InvalidInstanceId)
            {
                size_t Offset = m_PagesInstances.size();
#if NEON_DEBUG
                if (Offset >= NumberOfPages)
                {
                    std::unreachable();
                }
#endif
                auto& Page     = m_PagesInstances.emplace_back(Offset);
                auto  Instance = Page.Allocator.Allocate(1);
                InstanceId     = uint32_t(Offset << 16) | uint32_t(Instance.Offset);
                if (InstanceData)
                {
                    *InstanceData = Page.MappedInstances + Instance.Offset;
                }
            }

            return InstanceId;
        }

        /// <summary>
        /// Remove an instance data
        /// </summary>
        void RemoveInstance(
            uint32_t InstanceId)
        {
            uint32_t PageIndex = InstanceId >> 16;
            uint32_t Offset    = InstanceId & 0xFFFF;
            m_PagesInstances[PageIndex].Allocator.Free({ .Offset = Offset, .Size = 1 });
        }

        /// <summary>
        /// Get the instance data
        /// </summary>
        [[nodiscard]] InstanceData* GetInstanceData(
            uint32_t InstanceId)
        {
            uint32_t PageIndex = InstanceId >> 16;
            uint32_t Offset    = InstanceId & 0xFFFF;
            return m_PagesInstances[PageIndex].MappedInstances + Offset;
        }

        /// <summary>
        /// Get the instance data
        /// </summary>
        [[nodiscard]] const InstanceData* GetInstanceData(
            uint32_t InstanceId) const
        {
            uint32_t PageIndex = InstanceId >> 16;
            uint32_t Offset    = InstanceId & 0xFFFF;
            return m_PagesInstances[PageIndex].MappedInstances + Offset;
        }

        /// <summary>
        /// Get the instance resource's handle
        /// </summary>
        [[nodiscard]] RHI::GpuResourceHandle GetInstanceHandle(
            uint32_t InstanceId) const
        {
            uint32_t PageIndex = InstanceId >> 16;
            uint32_t Offset    = InstanceId & 0xFFFF;
            return m_PagesInstances[PageIndex].Instances->GetHandle(Offset * SizeOfInstanceData);
        }

    private:
        PagedInstanceBufferArray m_PagesInstances;
    };
} // namespace Neon::Scene