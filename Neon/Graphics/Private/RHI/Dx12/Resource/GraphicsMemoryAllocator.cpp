#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/GraphicsMemoryAllocator.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    GraphicsMemoryAllocator::BuddyBlock::BuddyBlock(
        Dx12ResourceStateManager&     StateManager,
        D3D12MA::Allocator*           GpuAllocator,
        IGlobalBufferPool::BufferType Type,
        size_t                        SizeOfBuffer) :
        Allocator(SizeOfBuffer),
        StateManager(StateManager)
    {
        IGpuResource::InitDesc InitDesc;
        MResourceFlags         Flags;
        GraphicsBufferType     BufferType;

        switch (Type)
        {
        case IGlobalBufferPool::BufferType::ReadOnly:
            Flags.Set(RHI::EResourceFlags::DenyShaderResource);
            InitDesc.InitialState.Set(EResourceState::CopyDest);
            InitDesc.Name = STR("GraphicsMemoryAllocator::ReabackBuffer");
            BufferType    = GraphicsBufferType::Readback;

            break;
        case IGlobalBufferPool::BufferType::ReadWrite:
        case IGlobalBufferPool::BufferType::ReadWriteGPUR:
        case IGlobalBufferPool::BufferType::ReadWriteGPURW:
            InitDesc.InitialState.Set(EResourceState::CopySource);
            BufferType = GraphicsBufferType::Upload;

            if (Type == IGlobalBufferPool::BufferType::ReadWrite)
            {
                Flags.Set(RHI::EResourceFlags::DenyShaderResource);
                InitDesc.Name = STR("GraphicsMemoryAllocator::UploadBuffer");
            }
            else if (Type == IGlobalBufferPool::BufferType::ReadWriteGPURW)
            {
                Flags.Set(RHI::EResourceFlags::AllowUnorderedAccess);
                InitDesc.Name = STR("GraphicsMemoryAllocator::UploadBuffer_RW");
            }
            else
            {
                InitDesc.Name = STR("GraphicsMemoryAllocator::UploadBuffer_R");
            }
            break;
        default:
            std::unreachable();
        }

        this->Buffer.reset(static_cast<Dx12GpuResource*>(
            IGpuResource::Create(
                ResourceDesc::Buffer(SizeOfBuffer, Flags, BufferType),
                InitDesc)));
    }

    GraphicsMemoryAllocator::BuddyBlock::~BuddyBlock()
    {
        Buffer->SilentRelease();
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

    void GraphicsMemoryAllocator::Shutdown()
    {
        std::scoped_lock BufferLock(m_PoolMutex);
        for (auto& Allocator : m_BufferAllocators)
        {
            Allocator.BufferPools.clear();
        }
    }

    D3D12MA::Allocator* GraphicsMemoryAllocator::GetMA() const
    {
        return m_Allocator.Get();
    }

    auto GraphicsMemoryAllocator::AllocateBuffer(
        IGlobalBufferPool::BufferType Type,
        size_t                        BufferSize,
        size_t                        Alignement) -> Handle
    {
        NEON_ASSERT(Alignement > 0);

        auto& Allocator = m_BufferAllocators[int(Type)];
        BufferSize      = Math::AlignUp(BufferSize, Alignement);

        std::scoped_lock BufferLock(m_PoolMutex);

        for (auto Iter = Allocator.BufferPools.begin(); Iter != Allocator.BufferPools.end(); Iter++)
        {
            if (auto Hndl = Iter->Allocator.Allocate(BufferSize, Alignement))
            {
                return {
                    .Resource = Iter->Buffer.get(),
                    .Offset   = Hndl.Offset,
                    .Size     = Hndl.Size,
                    .Type     = Type
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

        auto& Block         = Allocator.BufferPools.emplace_back(m_StateManager, GetMA(), Type, Allocator.SizeOfBuffer);
        auto [Offset, Size] = Block.Allocator.Allocate(BufferSize, Alignement);

        return {
            .Resource = Block.Buffer.get(),
            .Offset   = Offset,
            .Size     = Size,
            .Type     = Type
        };
    }

    void GraphicsMemoryAllocator::FreeBuffers(
        std::span<Handle> Handles)
    {
        std::scoped_lock BufferLock(m_PoolMutex);
        for (auto& Hndl : Handles)
        {
            bool  Exists    = false;
            auto& Allocator = m_BufferAllocators[int(Hndl.Type)];

            for (auto& Block : Allocator.BufferPools)
            {
                if (Hndl.Resource == Block.Buffer.get())
                {
                    Block.Allocator.Free({ .Offset = Hndl.Offset, .Size = Hndl.Size });
                    Exists = true;
                    break;
                }
            }
            NEON_ASSERT(Exists, "Tried to free a non-existant buffer");
        }
    }

    Dx12ResourceStateManager* RHI::GraphicsMemoryAllocator::GetStateManager()
    {
        return &m_StateManager;
    }
} // namespace Neon::RHI
