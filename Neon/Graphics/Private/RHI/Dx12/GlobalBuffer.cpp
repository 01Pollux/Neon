#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <RHI/GlobalBuffer.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    auto IGlobalBufferPool::Allocate(
        size_t                        Size,
        size_t                        Alignement,
        IGlobalBufferPool::BufferType Type) -> Handle
    {
        Handle Hndl{};
        if (Size) [[likely]]
        {
            auto Allocator    = Dx12RenderDevice::Get()->GetAllocator();
            auto BufferHandle = Allocator->AllocateBuffer(Type, Size, Alignement);

            Hndl.Buffer = BufferHandle.Resource,
            Hndl.Offset = BufferHandle.Offset,
            Hndl.Size   = BufferHandle.Size,
            Hndl.Type   = Type;
        }
        return Hndl;
    }

    void IGlobalBufferPool::Free(
        std::span<const Handle> Handles)
    {
        auto Allocator = Dx12RenderDevice::Get()->GetAllocator();

        for (auto& Hndl : Handles)
        {
            if (Hndl.Size) [[likely]]
            {
                Dx12Swapchain::Get()->SafeRelease(
                    { .Resource = Hndl.Buffer,
                      .Offset   = Hndl.Offset,
                      .Size     = Hndl.Size,
                      .Type     = Hndl.Type });
            }
        }
    }
} // namespace Neon::RHI