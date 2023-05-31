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

    const Size2I& IRenderPass::GetViewport() const
    {
        return m_Viewport;
    }

    void IRenderPass::SetViewport(
        const Size2I& Viewport)
    {
        m_Viewport = Viewport;
    }

    void IRenderPass::SetWindowViewport()
    {
        m_Viewport = {};
    }
} // namespace Neon::RG