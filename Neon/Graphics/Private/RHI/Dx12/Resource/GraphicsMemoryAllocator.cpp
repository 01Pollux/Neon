#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/GraphicsMemoryAllocator.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    GraphicsMemoryAllocator::BuddyBlock::BuddyBlock(
        Dx12ResourceStateManager& StateManager,
        D3D12MA::Allocator*       GpuAllocator,
        GraphicsBufferType        Type,
        D3D12_RESOURCE_FLAGS      Flags,
        size_t                    SizeOfBuffer) :
        Allocator(SizeOfBuffer),
        StateManager(StateManager)
    {
        D3D12_RESOURCE_STATES    InitialState;
        D3D12MA::ALLOCATION_DESC AllocDesc{};

        switch (Type)
        {
        case GraphicsBufferType::Default:
            InitialState       = D3D12_RESOURCE_STATE_COMMON;
            AllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
            break;
        case GraphicsBufferType::Upload:
            InitialState       = D3D12_RESOURCE_STATE_GENERIC_READ;
            AllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
            break;
        case GraphicsBufferType::Readback:
            InitialState       = D3D12_RESOURCE_STATE_COPY_DEST;
            AllocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
            Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
            break;
        default:
            std::unreachable();
        }

        CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
            SizeOfBuffer,
            Flags);

        ThrowIfFailed(GpuAllocator->CreateResource(
            &AllocDesc,
            &ResourceDesc,
            InitialState,
            nullptr,
            &this->Allocation,
            IID_PPV_ARGS(&Resource)));

        StateManager.StartTrakingResource(Resource.Get(), InitialState);
        RenameObject(Resource, STR("GraphicsMemoryAllocator::Buffer"));
    }

    GraphicsMemoryAllocator::BuddyBlock::~BuddyBlock()
    {
        StateManager.StopTrakingResource(Resource.Get());
    }

    RHI::GraphicsMemoryAllocator::GraphicsMemoryAllocator()
    {
        auto Dx12Device  = Dx12RenderDevice::Get()->GetDevice();
        auto Dx12Adapter = Dx12RenderDevice::Get()->GetAdapter();

        D3D12MA::ALLOCATOR_DESC Desc{
            .pDevice  = Dx12Device,
            .pAdapter = Dx12Adapter,
        };
        CreateAllocator(&Desc, &m_Allocator);
    }

    D3D12MA::Allocator* GraphicsMemoryAllocator::GetMA() const
    {
        return m_Allocator.Get();
    }

    GraphicsMemoryAllocator::Handle GraphicsMemoryAllocator::AllocateBuffer(
        GraphicsBufferType   Type,
        size_t               BufferSize,
        size_t               Alignement,
        D3D12_RESOURCE_FLAGS Flags)
    {
        BufferSize = Math::AlignUp(BufferSize, Alignement);

        auto&           Allocator = m_BufferAllocators[Flags][static_cast<size_t>(Type)];
        std::lock_guard BufferLock(Allocator.PoolMutex);

        for (auto Iter = Allocator.BufferPools.begin(); Iter != Allocator.BufferPools.end(); Iter++)
        {
            if (auto Hndl = Iter->Allocator.Allocate(BufferSize, Alignement))
            {
                return Handle{
                    .Resource = Iter->Resource,
                    .Offset   = Hndl.Offset,
                    .Size     = Hndl.Size,
                    .Type     = Type,
                    .Flags    = Flags
                };
            }
        }

        while (Allocator.SizeOfBuffer < BufferSize)
        {
            Allocator.SizeOfBuffer *= 2;
        }

        Allocator.SizeOfBuffer = Math::AlignUp(
            Allocator.SizeOfBuffer,
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

        auto& Block         = Allocator.BufferPools.emplace_back(m_StateManager, GetMA(), Type, Flags, Allocator.SizeOfBuffer);
        auto [Offset, Size] = Block.Allocator.Allocate(BufferSize, Alignement);

        return Handle{
            .Resource = Block.Resource,
            .Offset   = Offset,
            .Size     = Size,
            .Type     = Type,
            .Flags    = Flags
        };
    }

    void GraphicsMemoryAllocator::FreeBuffer(
        const Handle& Data)
    {
        auto&           Allocator = m_BufferAllocators[Data.Flags][static_cast<size_t>(Data.Type)];
        std::lock_guard BufferLock(Allocator.PoolMutex);

        for (auto& Block : Allocator.BufferPools)
        {
            if (Data.Resource == Block.Resource)
            {
                Block.Allocator.Free({ .Offset = Data.Offset, .Size = Data.Size });
                return;
            }
        }
        NEON_ASSERT(false, "Tried to free a non-existant buffer");
    }

    Dx12ResourceStateManager* RHI::GraphicsMemoryAllocator::GetStateManager()
    {
        return &m_StateManager;
    }
} // namespace Neon::RHI
