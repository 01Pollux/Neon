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

        static constexpr uint32_t InvalidInstanceId   = std::numeric_limits<uint32_t>::max();
        static constexpr size_t   SizeOfInstanceData  = sizeof(InstanceData);
        static constexpr uint32_t AlignOfInstanceData = uint32_t(alignof(InstanceData));
        static constexpr size_t   SizeOfPage          = std::numeric_limits<uint16_t>::max();
        static constexpr size_t   SizeOfInstancePage  = Math::AlignUp(SizeOfInstanceData * SizeOfPage, AlignOfInstanceData);
        static constexpr size_t   NumberOfPages       = std::numeric_limits<uint16_t>::max();

        using InstanceIdList          = std::vector<uint32_t>;
        using InstanceIdPipelineGroup = std::unordered_map<RHI::IPipelineState*, InstanceIdList>;

        using InstanceDataBuffer = Ptr<RHI::IGpuResource>;
        struct InstanceBufferPage
        {
            InstanceDataBuffer        Instances;
            Allocator::BuddyAllocator Allocator;
            uint8_t*                  MappedInstances;

            InstanceBufferPage(
                size_t PageIndex) :
                Instances(RHI::IGpuResource::Create(
                    RHI::ResourceDesc::BufferUpload(
                        SizeOfInstancePage,
                        {}),
                    {
#ifndef NEON_DIST
                        .Name = StringUtils::Format(STR("InstanceBufferPage_{}"), PageIndex).c_str()
#endif
                    })),
                Allocator(SizeOfPage),
                MappedInstances(Instances->Map())
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
            InstanceData** OutData = nullptr)
        {
            uint32_t InstanceId = InvalidInstanceId;

            for (uint32_t i = 0; i < uint32_t(m_PagesInstances.size()); i++)
            {
                auto& Page = m_PagesInstances[i];
                if (auto Instance = Page.Allocator.Allocate(1))
                {
                    InstanceId = uint32_t(i << 16) | uint32_t(Instance.Offset);
                    if (OutData)
                    {
                        *OutData = std::bit_cast<InstanceData*>(Page.MappedInstances + SizeOfInstanceData * Instance.Offset);
                    }
                }
            }

            if (InstanceId == InvalidInstanceId)
            {
                size_t PageIndex = m_PagesInstances.size();
#ifdef NEON_DEBUG
                if (PageIndex >= NumberOfPages)
                {
                    std::unreachable();
                }
#endif
                auto& Page     = m_PagesInstances.emplace_back(PageIndex);
                auto  Instance = Page.Allocator.Allocate(1);
                InstanceId     = uint32_t(PageIndex << 16) | uint32_t(Instance.Offset);
                if (OutData)
                {
                    *OutData = std::bit_cast<InstanceData*>(Page.MappedInstances + SizeOfInstanceData * Instance.Offset);
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
            return std::bit_cast<InstanceData*>(m_PagesInstances[PageIndex].MappedInstances + SizeOfInstanceData * Offset);
        }

        /// <summary>
        /// Get the instance data
        /// </summary>
        [[nodiscard]] const InstanceData* GetInstanceData(
            uint32_t InstanceId) const
        {
            uint32_t PageIndex = InstanceId >> 16;
            uint32_t Offset    = InstanceId & 0xFFFF;
            return std::bit_cast<const InstanceData*>(m_PagesInstances[PageIndex].MappedInstances + SizeOfInstanceData * Offset);
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