#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    Dx12GpuResource::Dx12GpuResource(
        ISwapchain* Swapchain) :
        m_OwningSwapchain(Swapchain)
    {
    }

    ID3D12Resource* Dx12GpuResource::GetResource() const
    {
        return m_Resource.Get();
    }

    D3D12MA::Allocation* Dx12GpuResource::GetAllocation() const
    {
        return m_Allocation.Get();
    }

    //

    IBuffer* IBuffer::Create(
        ISwapchain*       Swapchain,
        const BufferDesc& Desc)
    {
        return NEON_NEW Dx12Buffer(Swapchain, Desc, GraphicsBufferType::Default);
    }

    Dx12Buffer::Dx12Buffer(
        ISwapchain*        Swapchain,
        const BufferDesc&  Desc,
        GraphicsBufferType Type) :
        Dx12GpuResource(Swapchain)
    {
        auto Allocator = static_cast<Dx12Swapchain*>(m_OwningSwapchain)->GetAllocator();
        m_Buffer       = Allocator->AllocateBuffer(Type, Desc.Size, size_t(Desc.Alignment), CastResourceFlags(Desc.Flags));
        m_Resource     = m_Buffer.Resource;
        m_Alignement   = Desc.Alignment;
    }

    Dx12Buffer::~Dx12Buffer()
    {
        static_cast<Dx12Swapchain*>(m_OwningSwapchain)->SafeRelease(m_Buffer);
    }

    ResourceDesc Dx12Buffer::GetDesc() const
    {
        return ResourceDesc::Buffer(m_Buffer.Size, m_Alignement, CastResourceFlags(m_Buffer.Flags));
    }

    size_t Dx12Buffer::GetSize() const
    {
        return m_Buffer.Size;
    }

    GpuResourceHandle Dx12Buffer::GetHandle() const
    {
        return { m_Resource->GetGPUVirtualAddress() + m_Buffer.Offset };
    }
    //

    IUploadBuffer* IUploadBuffer::Create(
        ISwapchain*       Swapchain,
        const BufferDesc& Desc)
    {
        return NEON_NEW Dx12UploadBuffer(Swapchain, Desc, GraphicsBufferType::Upload);
    }

    Dx12UploadBuffer::Dx12UploadBuffer(
        ISwapchain*        Swapchain,
        const BufferDesc&  Desc,
        GraphicsBufferType Type) :
        Dx12Buffer(Swapchain, Desc, Type)
    {
    }

    uint8_t* Dx12UploadBuffer::Map()
    {
        void* MappedData = nullptr;
        ThrowIfFailed(m_Resource->Map(0, nullptr, &MappedData));
        return std::bit_cast<uint8_t*>(MappedData) + m_Buffer.Offset;
    }

    void Dx12UploadBuffer::Unmap()
    {
        m_Resource->Unmap(0, nullptr);
    }

    //

    IReadbackBuffer* IReadbackBuffer::Create(
        ISwapchain*       Swapchain,
        const BufferDesc& Desc)
    {
        return NEON_NEW Dx12ReadbackBuffer(Swapchain, Desc, GraphicsBufferType::Readback);
    }

    Dx12ReadbackBuffer::Dx12ReadbackBuffer(
        ISwapchain*        Swapchain,
        const BufferDesc&  Desc,
        GraphicsBufferType Type) :
        Dx12Buffer(Swapchain, Desc, Type)
    {
    }

    uint8_t* Dx12ReadbackBuffer::Map()
    {
        void* MappedData = nullptr;
        ThrowIfFailed(m_Resource->Map(0, nullptr, &MappedData));
        return std::bit_cast<uint8_t*>(MappedData) + m_Buffer.Offset;
    }

    void Dx12ReadbackBuffer::Unmap()
    {
        m_Resource->Unmap(0, nullptr);
    }

    //

    [[nodiscard]] ITexture* ITexture::Create(
        ISwapchain*         Swapchain,
        const ResourceDesc& Desc)
    {
        return NEON_NEW Dx12Texture(Swapchain, Desc);
    }

    Dx12Texture::Dx12Texture(
        ISwapchain*              Swapchain,
        const RHI::ResourceDesc& Desc) :
        Dx12GpuResource(Swapchain)
    {
        D3D12_RESOURCE_DESC Dx12Desc{
            .Width            = Desc.Width,
            .Height           = Desc.Height,
            .DepthOrArraySize = Desc.Depth,
            .MipLevels        = Desc.MipLevels,
            .Format           = CastFormat(Desc.Format),
            .SampleDesc       = { .Count = Desc.SampleCount, .Quality = Desc.SampleQuality },
            .Flags            = CastResourceFlags(Desc.Flags),
        };

        switch (Desc.Layout)
        {
        case ResourceLayout::Unknown:
            Dx12Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            break;
        case ResourceLayout::RowMajor:
            Dx12Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            break;
        case ResourceLayout::StandardSwizzle64KB:
            Dx12Desc.Layout = D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE;
            break;
        case ResourceLayout::UndefinedSwizzle64KB:
            Dx12Desc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
            break;
        default:
            NEON_ASSERT(false, "Invalid texture layout");
            break;
        }

        switch (Desc.Type)
        {
        case ResourceType::Texture1D:
            Dx12Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
            break;
        case ResourceType::Texture2D:
            Dx12Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            break;
        case ResourceType::Texture3D:
            Dx12Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            break;
        default:
            NEON_ASSERT(false, "Invalid texture type");
            break;
        }

        D3D12MA::ALLOCATION_DESC AllocDesc{};
        AllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_CLEAR_VALUE ClearValue, *ClearValuePtr = nullptr;
        if (Desc.ClearValue && (Dx12Desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)))
        {
            ClearValuePtr         = &ClearValue;
            ClearValuePtr->Format = CastFormat(Desc.Format);

            std::visit(
                VariantVisitor{
                    [ClearValuePtr](const Color4& Color)
                    {
                        ClearValuePtr->Color[0] = Color.r();
                        ClearValuePtr->Color[1] = Color.g();
                        ClearValuePtr->Color[2] = Color.b();
                        ClearValuePtr->Color[3] = Color.a();
                    },
                    [ClearValuePtr](ClearOperation::DepthStencil DS)
                    {
                        ClearValuePtr->DepthStencil.Depth   = DS.Depth;
                        ClearValuePtr->DepthStencil.Stencil = DS.Stencil;
                    } },
                Desc.ClearValue->Value);
            m_ClearValue = Desc.ClearValue;
        }

        auto Allocator = static_cast<Dx12Swapchain*>(m_OwningSwapchain)->GetAllocator()->GetMA();
        ThrowIfFailed(Allocator->CreateResource(
            &AllocDesc,
            &Dx12Desc,
            D3D12_RESOURCE_STATE_COMMON,
            ClearValuePtr,
            &m_Allocation,
            IID_PPV_ARGS(&m_Resource)));

        uint16_t MipLevels = 1;
        if (Dx12Desc.MipLevels == 0)
        {
            for (uint64_t Width = Dx12Desc.Width, Height = Dx12Desc.Height;
                 Width && Height;
                 Width >>= 1, Height >>= 1)
            {
                MipLevels++;
            }
        }
        else
        {
            MipLevels = Dx12Desc.MipLevels;
        }
        m_Dimensions = { int(Dx12Desc.Width),
                         int(Dx12Desc.Height),
                         int(Dx12Desc.DepthOrArraySize) };
        m_MipLevels  = Desc.MipLevels;

        auto Dx12StateManager = static_cast<Dx12ResourceStateManager*>(m_OwningSwapchain->GetStateManager());
        Dx12StateManager->StartTrakingResource(m_Resource.Get(), D3D12_RESOURCE_STATE_COMMON);
    }

    Dx12Texture::Dx12Texture(
        ISwapchain*                        Swapchain,
        Win32::ComPtr<ID3D12Resource>      Texture,
        D3D12_RESOURCE_STATES              InitialState,
        Win32::ComPtr<D3D12MA::Allocation> Allocation) :
        Dx12GpuResource(Swapchain)
    {
        m_Resource   = std::move(Texture);
        m_Allocation = std::move(Allocation);

        if (m_Resource)
        {
            auto Desc = m_Resource->GetDesc();

            m_Dimensions = { int(Desc.Width),
                             int(Desc.Height),
                             int(Desc.DepthOrArraySize) };
            m_MipLevels  = Desc.MipLevels;

            auto Dx12StateManager = static_cast<Dx12ResourceStateManager*>(m_OwningSwapchain->GetStateManager());
            Dx12StateManager->StartTrakingResource(m_Resource.Get(), InitialState);
        }
    }

    Dx12Texture::~Dx12Texture()
    {
        if (m_Resource)
        {
            auto Dx12StateManager = static_cast<Dx12ResourceStateManager*>(m_OwningSwapchain->GetStateManager());
            Dx12StateManager->StopTrakingResource(m_Resource.Get());
            static_cast<Dx12Swapchain*>(m_OwningSwapchain)->SafeRelease(m_Resource);
        }
    }

    ResourceDesc Dx12Texture::GetDesc() const
    {
        NEON_ASSERT(m_Resource);
        auto Dx12Desc = m_Resource->GetDesc();

        ResourceDesc Desc{
            .Width         = Dx12Desc.Width,
            .Height        = Dx12Desc.Height,
            .Depth         = Dx12Desc.DepthOrArraySize,
            .MipLevels     = Dx12Desc.MipLevels,
            .Alignment     = uint32_t(Dx12Desc.Alignment),
            .SampleCount   = Dx12Desc.SampleDesc.Count,
            .SampleQuality = Dx12Desc.SampleDesc.Quality,
            .Flags         = CastResourceFlags(Dx12Desc.Flags),
            .Format        = CastFormat(Dx12Desc.Format),
            .ClearValue    = m_ClearValue
        };
        switch (Dx12Desc.Layout)
        {
        case D3D12_TEXTURE_LAYOUT_UNKNOWN:
            Desc.Layout = ResourceLayout::Unknown;
            break;
        case D3D12_TEXTURE_LAYOUT_ROW_MAJOR:
            Desc.Layout = ResourceLayout::RowMajor;
            break;
        case D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE:
            Desc.Layout = ResourceLayout::StandardSwizzle64KB;
            break;
        case D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE:
            Desc.Layout = ResourceLayout::UndefinedSwizzle64KB;
            break;
        default:
            break;
        }

        switch (Dx12Desc.Dimension)
        {
        case D3D12_RESOURCE_DIMENSION_BUFFER:
            Desc.Type = ResourceType::Buffer;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            Desc.Type = ResourceType::Texture1D;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            Desc.Type = ResourceType::Texture2D;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            Desc.Type = ResourceType::Texture3D;
            break;
        }
        return Desc;
    }

    const Vector3DI& Dx12Texture::GetDimensions() const
    {
        return m_Dimensions;
    }

    uint16_t Dx12Texture::GetMipLevels() const
    {
        return m_MipLevels;
    }

} // namespace Neon::RHI