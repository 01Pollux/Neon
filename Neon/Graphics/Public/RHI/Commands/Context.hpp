#pragma once

#include <Core/Neon.hpp>
#include <RHI/Commands/List.hpp>
#include <list>

namespace Neon::RHI
{
    class ICommandQueue;
    class ICommandList;

    class CommandContext
    {
    public:
        CommandContext() = default;
        CommandContext(
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
        void Upload(
            bool Clear = true);

        /// <summary>
        /// Reset the command lists.
        /// </summary>
        void Reset();

        /// <summary>
        /// Get the number of command lists in the batch.
        /// </summary>
        [[nodiscard]] size_t Size() const noexcept;

    private:
        std::vector<ICommandList*> m_CommandLists;

        CommandQueueType m_Type;
    };

    template<CommandQueueType _Type>
    class TCommandContext : public CommandContext
    {
    public:
        // clang-format off
        using CommandListType = std::conditional_t<
            _Type == CommandQueueType::Graphics, IGraphicsCommandList,
              std::conditional_t<
                  _Type == CommandQueueType::Compute, IComputeCommandList,
              std::conditional_t<
                  _Type == CommandQueueType::Copy, ICopyCommandList,
                  nullptr_t>
            >
        >;
        // clang-format on

        explicit TCommandContext() :
            CommandContext(_Type)
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

    using GraphicsCommandContext = TCommandContext<CommandQueueType::Graphics>;
    using ComputeCommandContext  = TCommandContext<CommandQueueType::Compute>;
    using CopyCommandContext     = TCommandContext<CommandQueueType::Copy>;
} // namespace Neon::RHI
