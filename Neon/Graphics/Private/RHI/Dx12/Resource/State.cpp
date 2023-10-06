#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>
#include <Private/RHI/Dx12/Resource/Resource.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    IResourceStateManager* IResourceStateManager::Get()
    {
        return RHI::Dx12RenderDevice::Get()->GetStateManager();
    }

    Dx12ResourceStateManager* Dx12ResourceStateManager::Get()
    {
        return static_cast<Dx12ResourceStateManager*>(IResourceStateManager::Get());
    }

    void Dx12ResourceStateManager::TransitionResource(
        IGpuResource*  Resource,
        MResourceState NewState,
        uint32_t       SubresourceIndex)
    {
        auto Dx12Resource = GetDx12Resource(Resource);
        auto State        = CastResourceStates(NewState);
        auto Lock         = LockStates();

        auto& PendingStates         = m_ResoureStates.PendingStates[Dx12Resource];
        auto& CurrentResourceStates = GetCurrentStates_Internal(Dx12Resource);

        if (SubresourceIndex == Resource_AllSubresources)
        {
            PendingStates.reserve(CurrentResourceStates.size());
            for (size_t i = 0; i < CurrentResourceStates.size(); i++)
            {
                PendingStates.emplace_back(State, uint32_t(i));
            }
        }
        else
        {
            PendingStates.emplace_back(State, SubresourceIndex);
        }
    }

    void Dx12ResourceStateManager::TransitionResource(
        IGpuResource*            Resource,
        const SubresourceStates& NewStates)
    {
        auto Dx12Resource = GetDx12Resource(Resource);
        auto Lock         = LockStates();

        auto& PendingStates = m_ResoureStates.PendingStates[Dx12Resource];
        PendingStates.reserve(PendingStates.size() + NewStates.size());

        std::transform(
            NewStates.begin(),
            NewStates.end(),
            std::back_inserter(PendingStates),
            [](auto& Iter)
            {
                return Dx12SubresourceState{
                    .State            = CastResourceStates(Iter.second),
                    .SubresourceIndex = Iter.first
                };
            });
    }

    CommandContext Dx12ResourceStateManager::FlushBarriers()
    {
        if (auto Barriers = Flush(); !Barriers.empty())
        {
            CommandContext CtxBatch;

            auto CommandList = CtxBatch.Append();

            auto DxCommandList = static_cast<Dx12CommandList*>(CommandList)->Get();
            DxCommandList->ResourceBarrier(UINT(Barriers.size()), Barriers.data());

            return CtxBatch;
        }
        return {};
    }

    bool Dx12ResourceStateManager::FlushBarriers(
        ICommandList* CommandList)
    {
        if (auto Barriers = Flush(); !Barriers.empty())
        {
            auto DxCommandList = static_cast<Dx12CommandList*>(CommandList)->Get();
            DxCommandList->ResourceBarrier(UINT(Barriers.size()), Barriers.data());
            return true;
        }
        return false;
    }

    auto Dx12ResourceStateManager::GetCurrentStates(
        IGpuResource* Resource) -> SubresourceStates
    {
        auto Dx12Resource = GetDx12Resource(Resource);
        auto Lock         = LockStates();

        SubresourceStates Res;

        auto& CurrentStates = GetCurrentStates_Internal(Dx12Resource);
        for (auto& Iter : CurrentStates)
        {
            Res.emplace(Iter.SubresourceIndex, CastResourceStates(Iter.State));
        }

        return Res;
    }

    //

    void Dx12ResourceStateManager::StartTrakingResource(
        ID3D12Resource*       Resource,
        D3D12_RESOURCE_STATES InitialState)
    {
        auto Desc = Resource->GetDesc();

        uint32_t SubresourceCount = Desc.DepthOrArraySize * Desc.MipLevels;

        auto  Lock          = LockStates();
        auto& CurrentStates = m_ResoureStates.CurrentStates[Resource];
        CurrentStates.reserve(SubresourceCount);
        for (uint32_t i = 0; i < SubresourceCount; i++)
        {
            CurrentStates.emplace_back(InitialState, i);
        }
    }

    void Dx12ResourceStateManager::StopTrakingResource(
        ID3D12Resource* Resource)
    {
        auto Lock = LockStates();
        m_ResoureStates.CurrentStates.erase(Resource);
        m_ResoureStates.PendingStates.erase(Resource);
    }

    //

    auto Dx12ResourceStateManager::Flush() -> Dx12ResourceBarrierList
    {
        auto Lock = LockStates();

        Dx12ResourceBarrierList Barriers;
        for (auto& [Resource, States] : m_ResoureStates.PendingStates)
        {
            auto TempBarriers = TransitionToStatesImmediately(Resource, States);
            Barriers.insert(Barriers.begin(), TempBarriers.begin(), TempBarriers.end());
        }

        m_ResoureStates.PendingStates.clear();
        return Barriers;
    }

    auto Dx12ResourceStateManager::TransitionToStatesImmediately(
        ID3D12Resource*                 Resource,
        const Dx12SubresourceStateList& NewStates) -> Dx12ResourceBarrierList
    {
        auto  Lock                = LockStates();
        auto& CurrentSubresources = GetCurrentStates_Internal(Resource);

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
        ID3D12Resource* Resource) -> Dx12SubresourceStateList&
    {
        auto Iter = m_ResoureStates.CurrentStates.find(Resource);
        NEON_ASSERT(
            Iter != m_ResoureStates.CurrentStates.end(),
            "Resource is not registered / not being tracked. It may have been deallocated before transitions were applied");
        return Iter->second;
    }

    auto Dx12ResourceStateManager::TransitionToStatesImmediately_Internal(
        ID3D12Resource*                 Resource,
        const Dx12SubresourceStateList& NewStates) -> Dx12ResourceBarrierList
    {
        auto  Lock                = LockStates();
        auto& CurrentSubresources = GetCurrentStates_Internal(Resource);

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

    static constexpr D3D12_RESOURCE_STATES D3D12_READ_ONLY_STATES =
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER |
        D3D12_RESOURCE_STATE_INDEX_BUFFER |
        D3D12_RESOURCE_STATE_DEPTH_READ |
        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE |
        D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT |
        D3D12_RESOURCE_STATE_COPY_SOURCE;

    static constexpr D3D12_RESOURCE_STATES D3D12_WRITE_ONLY_STATES =
        D3D12_RESOURCE_STATE_RENDER_TARGET |
        D3D12_RESOURCE_STATE_STREAM_OUT |
        D3D12_RESOURCE_STATE_COPY_DEST;

    bool Dx12ResourceStateManager::IsNewStateRedundant(
        D3D12_RESOURCE_STATES CurrentState,
        D3D12_RESOURCE_STATES NewState)
    {
        // Transition is redundant if either states completely match
        // or current state is a read state and new state is a partial or complete subset of the current
        // (which implies that it is also a read state)
        return (CurrentState == NewState) ||
               ((CurrentState & D3D12_READ_ONLY_STATES) && ((CurrentState & NewState) == NewState));
    }
} // namespace Neon::RHI