#include <EnginePCH.hpp>
#include <Renderer/RenderGraph/Pass.hpp>

namespace Renderer::RG
{
    IRenderPass::IRenderPass(
        PassQueueType QueueType) noexcept :
        m_QueueType(QueueType)
    {
    }

    bool IRenderPass::OverrideViewport(const RenderGraphStorage&, RHI::GraphicsCommandContext&)
    {
        return false;
    }

    PassQueueType IRenderPass::GetQueueType() const
    {
        return m_QueueType;
    }

    const Size2I& IRenderPass::GetViewport() const
    {
        return m_Viewport;
    }

    void IRenderPass::SetViewport(const Size2I& Viewport)
    {
        m_Viewport = Viewport;
    }

    void IRenderPass::SetWindowViewport()
    {
        m_Viewport = {};
    }
} // namespace Renderer::RG
