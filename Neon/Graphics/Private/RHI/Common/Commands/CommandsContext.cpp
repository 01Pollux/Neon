#include <GraphicsPCH.hpp>
#include <RHI/Commands/Context.hpp>
#include <RHI/Commands/Queue.hpp>
#include <RHI/Swapchain.hpp>

namespace Neon::RHI
{
    CommandContext::CommandContext(
        ISwapchain*      Swapchain,
        CommandQueueType Type) :
        m_Swapchain(Swapchain),
        m_Type(Type)
    {
    }

    CommandContext::CommandContext(
        CommandContext&& Context) noexcept :
        m_Swapchain(Context.m_Swapchain),
        m_Type(Context.m_Type),
        m_CommandLists(std::exchange(Context.m_CommandLists, {}))
    {
    }

    CommandContext& CommandContext::operator=(
        CommandContext&& Context) noexcept
    {
        if (this != &Context)
        {
            m_Swapchain    = Context.m_Swapchain;
            m_CommandLists = std::exchange(Context.m_CommandLists, {});
        }
        return *this;
    }

    CommandContext::~CommandContext()
    {
        Upload();
    }

    size_t CommandContext::Append(
        size_t Size)
    {
        size_t Pos      = m_CommandLists.size();
        auto   Queue    = m_Swapchain->GetQueue(m_Type);
        auto   Commands = Queue->AllocateCommandLists(m_Type, Size);
        m_CommandLists.insert_range(m_CommandLists.end(), Commands);
        return Pos;
    }

    ICommandList* CommandContext::Append()
    {
        return (*this)[Append(1)];
    }

    ICommandList* CommandContext::operator[](
        size_t Index)
    {
        return m_CommandLists[Index];
    }

    void CommandContext::Upload(
        bool Clear)
    {
        if (!m_CommandLists.empty())
        {
            auto Queue = m_Swapchain->GetQueue(m_Type);
            Queue->Upload(m_CommandLists);
            if (Clear)
            {
                Queue->FreeCommandLists(m_Type, m_CommandLists);
                m_CommandLists.clear();
            }
        }
    }

    void CommandContext::Reset()
    {
        if (!m_CommandLists.empty())
        {
            auto Queue = m_Swapchain->GetQueue(m_Type);
            Queue->Reset(m_Type, m_CommandLists);
        }
    }
} // namespace Neon::RHI