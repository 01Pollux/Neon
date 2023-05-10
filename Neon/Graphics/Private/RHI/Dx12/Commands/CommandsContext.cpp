#include <GraphicsPCH.hpp>
#include <RHI/Commands/CommandsContext.hpp>
#include <RHI/Commands/CommandQueue.hpp>

namespace Neon::RHI
{
    CommandContext::CommandContext(
        ICommandQueue*   Queue,
        CommandQueueType Type) :
        m_Queue(Queue),
        m_CommandList(Queue->AllocateCommandList(Type))
    {
    }

    void CommandContext::Upload()
    {
        m_Queue->Upload(m_CommandList);
    }

    //

    CommandContextBatcher::CommandContextBatcher(
        ICommandQueue*   Queue,
        CommandQueueType Type) :
        m_Queue(Queue),
        m_Type(Type)
    {
    }

    ICommandList* CommandContextBatcher::Append()
    {
        return m_CommandLists.emplace_back(m_Queue->AllocateCommandList(m_Type));
    }

    ICommandList* CommandContextBatcher::operator[](
        size_t Index)
    {
        return m_CommandLists[Index];
    }

    void CommandContextBatcher::Append(
        size_t Size)
    {
        auto Commands = m_Queue->AllocateCommandLists(m_Type, Size);
        m_CommandLists.insert_range(m_CommandLists.end(), Commands);
    }

    void CommandContextBatcher::Upload()
    {
        if (!m_CommandLists.empty())
        {
            m_Queue->Upload(m_CommandLists);
        }
    }
} // namespace Neon::RHI