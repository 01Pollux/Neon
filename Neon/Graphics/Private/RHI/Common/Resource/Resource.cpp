#include <GraphicsPCH.hpp>
#include <RHI/Resource/Resource.hpp>
#include <RHI/Swapchain.hpp>

namespace Neon::RHI
{
    IGpuResource* PendingResource::Access(
        ICommandQueue* Queue) const
    {
        if (m_CopyId)
        {
            m_Resource->GetSwapchain()->WaitForCopy(
                Queue,
                *m_CopyId);
            m_CopyId = std::nullopt;
        }
        return m_Resource.get();
    }

    IGpuResource* PendingResource::Access(
        RHI::CommandQueueType QueueType) const
    {
        if (m_CopyId)
        {
            m_Resource->GetSwapchain()->WaitForCopy(
                m_Resource->GetSwapchain()->GetQueue(QueueType),
                *m_CopyId);
            m_CopyId = std::nullopt;
        }
        return m_Resource.get();
    }
} // namespace Neon::RHI