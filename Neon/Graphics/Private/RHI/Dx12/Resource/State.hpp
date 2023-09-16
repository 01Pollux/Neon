#pragma once

#include <GraphicsPCH.hpp>
#include <RHI/Resource/State.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <set>

namespace Neon::RHI
{
    class Dx12ResourceStateManager final : public IResourceStateManager
    {
    private:
        struct Dx12SubresourceState
        {
            D3D12_RESOURCE_STATES State            = D3D12_RESOURCE_STATE_COMMON;
            uint32_t              SubresourceIndex = 0;

            [[nodiscard]] std::strong_ordering operator<=>(
                const Dx12SubresourceState&) const noexcept = default;
        };

        using Dx12SubresourceStateList = std::vector<Dx12SubresourceState>;
        using Dx12SubresourceStateSet  = std::set<Dx12SubresourceState>;
        using Dx12ResourceBarrierList  = std::vector<D3D12_RESOURCE_BARRIER>;

        using ResourceStateInfoMapType = std::unordered_map<ID3D12Resource*, Dx12SubresourceStateList>;
        using ResourceStateMapType     = std::unordered_map<ID3D12Resource*, Dx12SubresourceStateList>;

        struct ResourceStateMapInfo
        {
            ResourceStateMapType CurrentStates;
            ResourceStateMapType PendingStates;
        };

    public:
        /// <summary>
        /// Get resource state manager instance
        /// </summary>
        [[nodiscard]] static Dx12ResourceStateManager* Get();

        void TransitionResource(
            IGpuResource*  Resource,
            MResourceState NewState,
            uint32_t       SubresourceIndex = Resource_AllSubresources) override;

        void TransitionResource(
            IGpuResource*            Resource,
            const SubresourceStates& NewStates) override;

        CommandContext FlushBarriers(
            CommandQueueType Type) override;

        bool FlushBarriers(
            ICommandList* CommandList) override;

        virtual SubresourceStates GetCurrentStates(
            IGpuResource* Resource) override;

    public:
        /// <summary>
        /// Start tracking resource's state
        /// </summary>
        void StartTrakingResource(
            ID3D12Resource*       Resource,
            D3D12_RESOURCE_STATES InitialState);

        /// <summary>
        /// Stop tracking resource's state
        /// </summary>
        void StopTrakingResource(
            ID3D12Resource* Resource);

    private:
        /// <summary>
        /// Flush pending state transitions
        /// </summary>
        [[nodiscard]] Dx12ResourceBarrierList Flush();

        /// <summary>
        /// Get current resource's states
        /// </summary>
        [[nodiscard]] Dx12ResourceBarrierList TransitionToStatesImmediately(
            ID3D12Resource*                 Resource,
            const Dx12SubresourceStateList& NewStates);

    private:
        /// <summary>
        /// Get current resource's states
        /// </summary>
        Dx12SubresourceStateList& GetCurrentStates_Internal(
            ID3D12Resource* Resource);

        /// <summary>
        /// Immediately record new state for a resource
        /// </summary>
        [[nodiscard]] Dx12ResourceBarrierList TransitionToStatesImmediately_Internal(
            ID3D12Resource*                 Resource,
            const Dx12SubresourceStateList& NewStates);

    private:
        /// <summary>
        /// Transition is redundant if either states completely match
        /// or current state is a read state and new state is a partial or complete subset of the current
        /// (which implies that it is also a read state)
        /// </summary>
        [[nodiscard]] static bool IsNewStateRedundant(
            D3D12_RESOURCE_STATES CurrentState,
            D3D12_RESOURCE_STATES NewState);

        /// <summary>
        /// Lock resource states mutex
        /// </summary>
        [[nodiscard]] auto LockStates()
        {
            return std::scoped_lock{ m_StatesMutex };
        }

    private:
        std::recursive_mutex m_StatesMutex;
        ResourceStateMapInfo m_ResoureStates;
    };
} // namespace Neon::RHI