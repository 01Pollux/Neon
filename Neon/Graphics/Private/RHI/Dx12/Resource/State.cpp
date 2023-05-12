#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>
#include <Private/RHI/Dx12/Resource/Resource.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    void Dx12ResourceStateManager::TransitionResource(
        IGpuResource*  Resource,
        MResourceState NewState,
        uint32_t       SubresourceIndex)
    {
        auto Dx12Resource = GetDx12Resource(Resource);
        auto States       = m_ResoureStates.Lock();

        auto& PendingStates         = States->PendingStates[Dx12Resource];
        auto& CurrentResourceStates = GetCurrentStates_Internal(Dx12Resource, *States);

        if (SubresourceIndex == Resource_AllSubresources)
        {
            PendingStates.reserve(CurrentResourceStates.size());
            for (size_t i = 0; i < CurrentResourceStates.size(); i++)
            {
                PendingStates.emplace_back(CastResourceStates(NewState), uint32_t(i));
            }
        }
        else
        {
            PendingStates.emplace_back(CastResourceStates(NewState), SubresourceIndex);
        }
    }

    void Dx12ResourceStateManager::TransitionResource(
        IGpuResource*            Resource,
        const SubresourceStates& NewStates)
    {
        auto Dx12Resource = GetDx12Resource(Resource);
        auto States       = m_ResoureStates.Lock();

        auto& PendingStates = States->PendingStates[Dx12Resource];
        std::transform(
            NewStates.begin(),
            NewStates.end(),
            PendingStates.end(),
            [](auto& Iter)
            {
                return Dx12SubresourceState{
                    .State            = CastResourceStates(Iter.second),
                    .SubresourceIndex = Iter.first
                };
            });
    }

    CommandContext Dx12ResourceStateManager::FlushBarriers(
        ICommandQueue* Queue)
    {
        if (auto Barriers = Flush(); !Barriers.empty())
        {
            TCommandContext<CommandQueueType::Graphics> Context(Queue);

            auto Dx12CommandList = dynamic_cast<Dx12GraphicsCommandList*>(Context.Get())->Get();
            Dx12CommandList->ResourceBarrier(UINT(Barriers.size()), Barriers.data());

            return Context;
        }
        return {};
    }

    void Dx12ResourceStateManager::FlushBarriers(
        ICommandList* CommandList)
    {
        if (auto Barriers = Flush(); !Barriers.empty())
        {
            auto Dx12CommandList = dynamic_cast<Dx12GraphicsCommandList*>(CommandList)->Get();
            Dx12CommandList->ResourceBarrier(UINT(Barriers.size()), Barriers.data());
        }
    }

    auto Dx12ResourceStateManager::GetCurrentStates(
        IGpuResource* Resource) -> SubresourceStates
    {
        auto Dx12Resource = GetDx12Resource(Resource);
        auto States       = m_ResoureStates.Lock();

        SubresourceStates Res;

        auto& CurrentStates = GetCurrentStates_Internal(Dx12Resource, *States);
        std::for_each(
            CurrentStates.begin(),
            CurrentStates.end(),
            [&Res](auto& Iter)
            {
                Res.emplace(Iter.SubresourceIndex, CastResourceStates(Iter.State));
            });

        return Res;
    }

    //

    void Dx12ResourceStateManager::StartTrakingResource(
        ID3D12Resource*       Resource,
        D3D12_RESOURCE_STATES InitialState)
    {
        auto States = m_ResoureStates.Lock();
        auto Desc   = Resource->GetDesc();

        uint32_t SubresourceCount = Desc.DepthOrArraySize * Desc.MipLevels;

        auto& CurrentStates = States->CurrentStates[Resource];
        CurrentStates.reserve(SubresourceCount);

        for (uint32_t i = 0; i < SubresourceCount; i++)
        {
            CurrentStates.emplace_back(InitialState, i);
        }
    }

    void Dx12ResourceStateManager::StopTrakingResource(
        ID3D12Resource* Resource)
    {
        auto States = m_ResoureStates.Lock();
        States->CurrentStates.erase(Resource);
        States->PendingStates.erase(Resource);
    }

    //

    auto Dx12ResourceStateManager::Flush() -> Dx12ResourceBarrierList
    {
        auto States = m_ResoureStates.Lock();

        Dx12ResourceBarrierList Barriers;
        for (auto& [Resource, States] : States->PendingStates)
        {
            auto TempBarriers = TransitionToStatesImmediately(Resource, States);
            Barriers.insert(Barriers.begin(), TempBarriers.begin(), TempBarriers.end());
        }

        States->PendingStates.clear();
        return Barriers;
    }

    auto Dx12ResourceStateManager::TransitionToStatesImmediately(
        ID3D12Resource*                 Resource,
        const Dx12SubresourceStateList& NewStates) -> Dx12ResourceBarrierList
    {
        auto States = m_ResoureStates.Lock();

        auto& CurrentSubresources = GetCurrentStates_Internal(Resource, *States);

        Dx12ResourceBarrierList NewStateBarriers;

        bool StatesMatch = true;

        auto FirstOldState = CurrentSubresources.front().State;
        auto FirstNewState = NewStates.front().State;

        for (auto& NewSubresources : NewStates)
        {
            auto& CurrentState = CurrentSubresources[NewSubresources.SubresourceIndex];
            auto  OldState     = CurrentState.State;
            auto  NewState     = NewSubresources.State;

            if (IsNewStateRedundant(OldState, NewState))
            {
                continue;
            }

            CurrentState.State = NewSubresources.State;

            NewStateBarriers.emplace_back(
                CD3DX12_RESOURCE_BARRIER::Transition(
                    Resource,
                    OldState,
                    NewState,
                    NewSubresources.SubresourceIndex));

            // If any old subresource states do not match or any of the new states do not match
            // then performing single transition barrier for all subresources is not possible
            if (OldState != FirstOldState || NewState != FirstNewState)
            {
                StatesMatch = false;
            }
        }

        // If multiple transitions were requested, but it's possible to make just one - do it
        if (StatesMatch && NewStateBarriers.size() > 1)
        {
            NewStateBarriers.resize(1);
            NewStateBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        }

        return NewStateBarriers;
    }

    //

    auto Dx12ResourceStateManager::GetCurrentStates_Internal(
        ID3D12Resource*       Resource,
        ResourceStateMapInfo& States) -> Dx12SubresourceStateList&
    {
        auto Iter = States.CurrentStates.find(Resource);
        NEON_ASSERT(
            Iter != States.CurrentStates.end(),
            "Resource is not registered / not being tracked. It may have been deallocated before transitions were applied");
        return Iter->second;
    }

    auto Dx12ResourceStateManager::TransitionToStatesImmediately_Internal(
        ID3D12Resource*                 Resource,
        const Dx12SubresourceStateList& NewStates) -> Dx12ResourceBarrierList
    {
        auto States = m_ResoureStates.Lock();

        auto& CurrentSubresources = GetCurrentStates_Internal(Resource, *States);

        Dx12ResourceBarrierList NewStateBarriers;

        bool StatesMatch = true;

        auto FirstOldState = CurrentSubresources.front().State;
        auto FirstNewState = NewStates.front().State;

        for (const auto& NewSubresources : NewStates)
        {
            auto& CurrentState = CurrentSubresources[NewSubresources.SubresourceIndex];
            auto  OldState     = CurrentState.State;
            auto  NewState     = NewSubresources.State;

            if (IsNewStateRedundant(OldState, NewState))
            {
                continue;
            }

            CurrentState.State = NewSubresources.State;

            NewStateBarriers.emplace_back(
                CD3DX12_RESOURCE_BARRIER::Transition(
                    Resource,
                    OldState,
                    NewState,
                    NewSubresources.SubresourceIndex));

            // If any old subresource states do not match or any of the new states do not match
            // then performing single transition barrier for all subresources is not possible
            if (OldState != FirstOldState || NewState != FirstNewState)
            {
                StatesMatch = false;
            }
        }

        // If multiple transitions were requested, but it's possible to make just one - do it
        if (StatesMatch && NewStateBarriers.size() > 1)
        {
            NewStateBarriers.resize(1);
            NewStateBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        }

        return NewStateBarriers;
    }

    //

    bool Dx12ResourceStateManager::IsNewStateRedundant(
        D3D12_RESOURCE_STATES CurrentState,
        D3D12_RESOURCE_STATES NewState)
    {
        auto HasReadState = [](D3D12_RESOURCE_STATES State)
        {
            return State & (D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE |
                            D3D12_RESOURCE_STATE_GENERIC_READ |
                            D3D12_RESOURCE_STATE_PREDICATION |
                            D3D12_RESOURCE_STATE_DEPTH_READ);
        };
        // Transition is redundant if either states completely match
        // or current state is a read state and new state is a partial or complete subset of the current
        // (which implies that it is also a read state)
        return (CurrentState == NewState) ||
               (HasReadState(CurrentState) && ((CurrentState & NewState) == NewState));
    }
} // namespace Neon::RHI