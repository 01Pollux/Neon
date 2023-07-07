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
            RHI::ISwapchain::Get()->WaitForCopy(
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
            RHI::ISwapchain::Get()->WaitForCopy(
                RHI::ISwapchain::Get()->GetQueue(QueueType),
                *m_CopyId);
            m_CopyId = std::nullopt;
        }
        return m_Resource.get();
    }
} // namespace Neon::RHI