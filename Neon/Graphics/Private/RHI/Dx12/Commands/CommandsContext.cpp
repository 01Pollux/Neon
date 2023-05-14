#include <GraphicsPCH.hpp>
#include <RHI/Commands/CommandsContext.hpp>
#include <RHI/Commands/CommandQueue.hpp>

namespace Neon::RHI
{
    CommandContext::CommandContext(
        ICommandQueue*   Queue,
        CommandQueueType Type) :
        m_Queue(Queue),
        m_Type(Type)
    {
    }

    CommandContext::CommandContext(
        CommandContext&& Context) noexcept :
        m_Queue(Context.m_Queue),
        m_Type(Context.m_Type),
        m_CommandLists(std::exchange(Context.m_CommandLists, {}))
    {
    }

    CommandContext& CommandContext::operator=(
        CommandContext&& Context) noexcept
    {
        if (this != &Context)
        {
            m_Queue        = Context.m_Queue;
            m_CommandLists = std::exchange(Context.m_CommandLists, {});
        }
        return *this;
    }

    CommandContext::~CommandContext()
    {
        if (!m_CommandLists.empty())
        {
            m_Queue->FreeCommandLists(m_Type, m_CommandLists);
        }
    }

    size_t CommandContext::Append(
        size_t Size)
    {
        size_t Pos      = m_CommandLists.size();
        auto   Commands = m_Queue->AllocateCommandLists(m_Type, Size);
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

    void CommandContext::Upload()
    {
        if (!m_CommandLists.empty())
        {
            m_Queue->Upload(m_CommandLists);
        }
    }

    void CommandContext::Reset()
    {
        if (!m_CommandLists.empty())
        {
            m_Queue->Reset(m_Type, m_CommandLists);
        }
    }
} // namespace Neon::RHI