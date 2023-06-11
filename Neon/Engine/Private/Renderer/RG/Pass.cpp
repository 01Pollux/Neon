#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>

namespace Neon::RG
{
    IRenderPass::IRenderPass(
        PassQueueType QueueType) :
        m_QueueType(QueueType)
    {
    }

    PassQueueType IRenderPass::GetQueueType() const noexcept
    {
        return m_QueueType;
    }
} // namespace Neon::RG