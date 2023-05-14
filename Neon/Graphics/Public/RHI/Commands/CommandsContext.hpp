#pragma once

#include <Core/Neon.hpp>
#include <RHI/Commands/CommandList.hpp>
#include <list>

namespace Neon::RHI
{
    class ICommandQueue;
    class ICommandList;
    class IGraphicsCommandList;

    class CommandContext
    {
    public:
        CommandContext() = default;
        CommandContext(
            ISwapchain*      Swapchain,
            CommandQueueType Type);

        NEON_CLASS_NO_COPY(CommandContext);
        CommandContext(
            CommandContext&& Context) noexcept;
        CommandContext& operator=(
            CommandContext&& Context) noexcept;

        ~CommandContext();

        /// <summary>
        /// Add new command lists to the batch.
        /// </summary>
        size_t Append(
            size_t Size);

        /// <summary>
        /// Add new command lists to the batch.
        /// </summary>
        ICommandList* Append();

        /// <summary>
        /// Get a command list in the batch.
        /// </summary>
        [[nodiscard]] ICommandList* operator[](
            size_t Index);

        /// <summary>
        /// Submit list of command lists to the GPU.
        /// </summary>
        void Upload();

        /// <summary>
        /// Reset the command lists.
        /// </summary>
        void Reset();

    private:
        ISwapchain*      m_Swapchain;
        CommandQueueType m_Type;

        std::vector<ICommandList*> m_CommandLists;
    };

    template<CommandQueueType _Type>
    class TCommandContext : public CommandContext
    {
    public:
        using CommandListType = std::conditional_t<
            _Type == CommandQueueType::Graphics, IGraphicsCommandList,
            nullptr_t>;

        TCommandContext() = default;

        explicit TCommandContext(
            ISwapchain* Swapchain) :
            CommandContext(Swapchain, _Type)
        {
        }

        /// <summary>
        /// Add new command lists to the batch.
        /// </summary>
        CommandListType* Append()
        {
            return dynamic_cast<CommandListType*>(CommandContext::Append());
        }

        /// <summary>
        /// Get a command list in the batch.
        /// </summary>
        [[nodiscard]] CommandListType* operator[](
            size_t Index)
        {
            return dynamic_cast<CommandListType*>(CommandContext::operator[](Index));
        }
    };
} // namespace Neon::RHI
