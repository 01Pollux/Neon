#pragma once

#include <RHI/Resource/Resource.hpp>
#include <RHI/Commands/Context.hpp>

namespace Neon::RHI
{
    class ICommandList;

    class IResourceStateManager
    {
    public:
        using SubresourceStates = std::map<uint32_t, MResourceState>;

    public:
        virtual ~IResourceStateManager() = default;

        /// <summary>
        /// Transition a resource to a new state.
        /// </summary>
        virtual void TransitionResource(
            IGpuResource*  Resource,
            MResourceState NewState,
            uint32_t       SubresourceIndex = Resource_AllSubresources) = 0;

        /// <summary>
        /// Transition a resource to a new state.
        /// </summary>
        virtual void TransitionResource(
            IGpuResource*            Resource,
            const SubresourceStates& NewStates) = 0;

        /// <summary>
        /// Flush all the pending barriers into newly created command list.
        /// </summary>
        virtual CommandContext FlushBarriers(
            ISwapchain* Swapchain) = 0;

        /// <summary>
        /// Flush all the pending barriers into the given command list.
        /// </summary>
        virtual void FlushBarriers(
            ICommandList* CommandList) = 0;

        /// <summary>
        /// Get current resource's states
        /// </summary>
        [[nodiscard]] virtual SubresourceStates GetCurrentStates(
            IGpuResource* Resource) = 0;
    };
} // namespace Neon::RHI