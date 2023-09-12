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
    };
} // namespace Neon::RHI
