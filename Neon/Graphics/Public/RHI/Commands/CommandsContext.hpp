#pragma once

#include <RHI/Commands/Commands.hpp>
#include <list>

namespace Neon::RHI
{
    class ICommandQueue;
    class ICommandList;
    class IGraphicsCommandList;

    class CommandContext
    {
    public:
        CommandContext(
            ICommandQueue*   Queue,
            CommandQueueType Type);

        [[nodiscard]] ICommandList* operator->()
        {
            return m_CommandList;
        }

        [[nodiscard]] ICommandList* Get()
        {
            return m_CommandList;
        }

        /// <summary>
        /// Submit the command list to the GPU.
        /// </summary>
        void Upload();

    protected:
        ICommandQueue* m_Queue;
        ICommandList*  m_CommandList;
    };

    template<CommandQueueType _Type>
    class TCommandContext : public CommandContext
    {
    public:
        using CommandListType = std::conditional_t<
            _Type == CommandQueueType::Graphics, IGraphicsCommandList,
            nullptr_t>;

        TCommandContext(
            ICommandQueue* Queue) :
            CommandContext(Queue, _Type)
        {
        }

        [[nodiscard]] CommandListType* operator->()
        {
            return dynamic_cast<CommandListType*>(m_CommandList);
        }

        [[nodiscard]] CommandListType* Get()
        {
            return dynamic_cast<CommandListType*>(m_CommandList);
        }
    };

    class CommandContextBatcher
    {
    public:
        CommandContextBatcher(
            ICommandQueue*   Queue,
            CommandQueueType Type);

        /// <summary>
        /// Add a new command list to the batch.
        /// </summary>
        [[nodiscard]] ICommandList* Append();

        /// <summary>
        /// Get a command list in the batch.
        /// </summary>
        [[nodiscard]] ICommandList* operator[](
            size_t Index);

        /// <summary>
        /// Add new command lists to the batch.
        /// </summary>
        void Append(
            size_t Size);

        /// <summary>
        /// Submit list of command lists to the GPU.
        /// </summary>
        void Upload();

    private:
        ICommandQueue*   m_Queue;
        CommandQueueType m_Type;

        std::vector<ICommandList*> m_CommandLists;
    };
} // namespace Neon::RHI
