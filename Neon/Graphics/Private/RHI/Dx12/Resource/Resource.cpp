#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Resource/TextureLoader.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    void Dx12GpuResource::QueryFootprint(
        uint32_t              FirstSubresource,
        uint32_t              SubresourceCount,
        size_t                Offset,
        SubresourceFootprint* OutFootprint,
        uint32_t*             NumRows,
        size_t*               RowSizeInBytes,
        size_t*               LinearSize) const
    {
        // TODO: Use this->GetDx12Desc(); instead (convert to D3D12_RESOURCE_DESC), since we know the resource
        auto Desc = m_Resource->GetDesc();

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint{};

        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        Dx12Device->GetCopyableFootprints(
            &Desc,
            FirstSubresource,
            SubresourceCount,
            Offset,
            OutFootprint ? &Footprint : nullptr,
            NumRows,
            RowSizeInBytes,
            LinearSize);

        if (OutFootprint)
        {
            OutFootprint->Offset   = Footprint.Offset;
            OutFootprint->Format   = CastFormat(Footprint.Footprint.Format);
            OutFootprint->Width    = Footprint.Footprint.Width;
            OutFootprint->Height   = Footprint.Footprint.Height;
            OutFootprint->Depth    = Footprint.Footprint.Depth;
            OutFootprint->RowPitch = Footprint.Footprint.RowPitch;
        }
    }

    void Dx12GpuResource::CopyFrom(
        uint32_t                         FirstSubresource,
        std::span<const SubresourceDesc> Subresources,
        uint64_t&                        CopyId)
    {
        struct SubresourceDescGuard
        {
            std::vector<SubresourceDesc>            Subresources;
            std::vector<std::unique_ptr<uint8_t[]>> Datas;

            SubresourceDescGuard() = default;
            NEON_CLASS_NO_COPY(SubresourceDescGuard);
            NEON_CLASS_MOVE(SubresourceDescGuard);
            ~SubresourceDescGuard() = default;
        };

        auto Guard = std::make_shared<SubresourceDescGuard>();

        Guard->Subresources = Subresources |
                              std::ranges::to<std::vector<SubresourceDesc>>();
        Guard->Datas.reserve(Subresources.size());
        for (auto& Subresource : Guard->Subresources)
        {
            size_t Size    = Subresource.SlicePitch;
            auto   NewData = Guard->Datas.emplace_back(std::make_unique<uint8_t[]>(Size)).get();
            std::copy_n(std::bit_cast<uint8_t*>(Subresource.Data), Size, std::bit_cast<uint8_t*>(NewData));
            Subresource.Data = NewData;
        }

        CopyId = Dx12Swapchain::Get()->RequestCopy(
            [SubreourcesGuard = std::move(Guard)](ICopyCommandList* CommandList,
                                                  Dx12GpuResource*  Resource)
            {
                size_t TotalBytes;
                Resource->QueryFootprint(
                    0,
                    uint32_t(SubreourcesGuard->Subresources.size()),
                    0,
                    nullptr,
                    nullptr,
                    nullptr,
                    &TotalBytes);

                UBufferPoolHandle Handle(
                    TotalBytes,
                    1,
                    RHI::IGlobalBufferPool::BufferType::ReadWrite);

                CommandList->CopySubresources(
                    Resource,
                    static_cast<IGpuResource*>(Handle.Buffer),
                    Handle.Offset,
                    0,
                    SubreourcesGuard->Subresources);
            },
            this);
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
        const BufferDesc& Desc)
    {
        return NEON_NEW Dx12Buffer(Desc, nullptr, nullptr, GraphicsBufferType::Default);
    }

    IBuffer* IBuffer::Create(
        const BufferDesc&      Desc,
        const SubresourceDesc& Subresource,
        uint64_t&              CopyId)
    {
        return NEON_NEW Dx12Buffer(Desc, &Subresource, &CopyId, GraphicsBufferType::Default);
    }

    Dx12Buffer::Dx12Buffer(
        const BufferDesc&      Desc,
        const SubresourceDesc* Subresource,
        uint64_t*              CopyId,
        GraphicsBufferType     Type) :
        Dx12Buffer(Desc.Size, CastResourceFlags(Desc.Flags), Type)
    {
        if (Subresource)
        {
            CopyFrom(0, { Subresource, 1 }, *CopyId);
        }
    }

    Dx12Buffer::Dx12Buffer(
        size_t               Size,
        D3D12_RESOURCE_FLAGS Flags,
        GraphicsBufferType   Type) :
        m_Type(Type)
    {
        auto Allocator = Dx12RenderDevice::Get()->GetAllocator();

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

        auto Dx12Desc = CD3DX12_RESOURCE_DESC::Buffer(Size, Flags, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
        m_Desc        = ResourceDesc::Buffer(Dx12Desc.Width, CastResourceFlags(Flags));

        ThrowIfFailed(Allocator->GetMA()->CreateResource(
            &AllocDesc,
            &Dx12Desc,
            InitialState,
            nullptr,
            &m_Allocation,
            IID_PPV_ARGS(&m_Resource)));

        Dx12RenderDevice::Get()->GetDevice()->GetCopyableFootprints(
            &Dx12Desc,
            0,
            1,
            0,
            nullptr,
            nullptr,
            nullptr,
            &m_Desc.Width);

        Dx12ResourceStateManager::Get()->StartTrakingResource(m_Resource.Get(), InitialState);
    }

    Dx12Buffer::~Dx12Buffer()
    {
        Dx12ResourceStateManager::Get()->StopTrakingResource(m_Resource.Get());
        Dx12Swapchain::Get()->SafeRelease(m_Resource, m_Allocation);
    }

    size_t Dx12Buffer::GetSize() const
    {
        return m_Desc.Width;
    }

    GpuResourceHandle Dx12Buffer::GetHandle(
        size_t Offset) const
    {
        return { m_Resource->GetGPUVirtualAddress() + GetOffset() + Offset };
    }

    size_t Dx12Buffer::GetOffset() const
    {
        return IsUsingPool() ? m_Offset : 0;
    }

    bool Dx12Buffer::IsUsingPool() const
    {
        return m_Allocation == nullptr;
    }
    //

    IUploadBuffer* IUploadBuffer::Create(
        const BufferDesc& Desc)
    {
        return NEON_NEW Dx12UploadBuffer(Desc, nullptr, nullptr);
    }

    IUploadBuffer* IUploadBuffer::Create(
        const BufferDesc&      Desc,
        const SubresourceDesc& Subresource,
        uint64_t&              CopyId)
    {
        return NEON_NEW Dx12UploadBuffer(Desc, &Subresource, &CopyId);
    }

    Dx12UploadBuffer::Dx12UploadBuffer(
        const BufferDesc&      Desc,
        const SubresourceDesc* Subresource,
        uint64_t*              CopyId) :
        Dx12Buffer(Desc, Subresource, CopyId, GraphicsBufferType::Upload)
    {
    }

    Dx12UploadBuffer::Dx12UploadBuffer(
        size_t               Size,
        D3D12_RESOURCE_FLAGS Flags) :
        Dx12Buffer(Size, Flags, GraphicsBufferType::Upload)
    {
    }

    uint8_t* Dx12UploadBuffer::Map()
    {
        void* MappedData = nullptr;
        ThrowIfFailed(m_Resource->Map(0, nullptr, &MappedData));
        return std::bit_cast<uint8_t*>(MappedData) + GetOffset();
    }

    void Dx12UploadBuffer::Unmap()
    {
        m_Resource->Unmap(0, nullptr);
    }

    //

    IReadbackBuffer* IReadbackBuffer::Create(
        const BufferDesc& Desc)
    {
        return NEON_NEW Dx12ReadbackBuffer(Desc);
    }

    Dx12ReadbackBuffer::Dx12ReadbackBuffer(
        const BufferDesc& Desc) :
        Dx12Buffer(Desc, nullptr, nullptr, GraphicsBufferType::Readback)
    {
    }

    Dx12ReadbackBuffer::Dx12ReadbackBuffer(
        size_t               Size,
        D3D12_RESOURCE_FLAGS Flags) :
        Dx12Buffer(Size, Flags, GraphicsBufferType::Readback)
    {
    }

    uint8_t* Dx12ReadbackBuffer::Map()
    {
        void* MappedData = nullptr;
        ThrowIfFailed(m_Resource->Map(0, nullptr, &MappedData));
        return std::bit_cast<uint8_t*>(MappedData) + GetOffset();
    }

    void Dx12ReadbackBuffer::Unmap()
    {
        m_Resource->Unmap(0, nullptr);
    }

    //

    ITexture* ITexture::Create(
        const ResourceDesc& Desc)
    {
        return NEON_NEW Dx12Texture(Desc, {}, nullptr);
    }

    ITexture* ITexture::Create(
        const ResourceDesc&              Desc,
        std::span<const SubresourceDesc> Subresources,
        uint64_t&                        CopyId)
    {
        return NEON_NEW Dx12Texture(Desc, Subresources, &CopyId);
    }

    ITexture* ITexture::Create(
        const TextureRawImage& ImageData,
        uint64_t&              CopyId)
    {
        if (!ImageData.Data)
        {
            return nullptr;
        }

        using TextureLoadFuncType    = decltype(&TextureLoader::LoadDDS);
        TextureLoadFuncType LoadFunc = nullptr;

        switch (ImageData.Type)
        {
        case TextureRawImage::Format::Dds:
        {
            LoadFunc = &TextureLoader::LoadDDS;
            break;
        }
        case TextureRawImage::Format::Bmp:
        case TextureRawImage::Format::Ico:
        case TextureRawImage::Format::Png:
        case TextureRawImage::Format::Jpeg:
        case TextureRawImage::Format::Jxr:
        case TextureRawImage::Format::Tiff:
        {
            LoadFunc = &TextureLoader::LoadWIC;
            break;
        }

        default:
            NEON_ASSERT(false, "Tried to load an unsupported image format");
            std::unreachable();
        }

        auto Image = LoadFunc(ImageData.Data, ImageData.Size);
        CopyId     = Image.GetUploadId();
        return Image.Release();
    }

    Dx12Texture::Dx12Texture(
        const RHI::ResourceDesc&         Desc,
        std::span<const SubresourceDesc> Subresources,
        uint64_t*                        CopyId)
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
            ClearValuePtr->Format = CastFormat(Desc.ClearValue->Format);

            std::visit(
                VariantVisitor{
                    [ClearValuePtr](const Color4& Color)
                    {
                        ClearValuePtr->Color[0] = Color.r;
                        ClearValuePtr->Color[1] = Color.g;
                        ClearValuePtr->Color[2] = Color.b;
                        ClearValuePtr->Color[3] = Color.a;
                    },
                    [ClearValuePtr](ClearOperation::DepthStencil DS)
                    {
                        ClearValuePtr->DepthStencil.Depth   = DS.Depth;
                        ClearValuePtr->DepthStencil.Stencil = DS.Stencil;
                    } },
                Desc.ClearValue->Value);
            m_ClearValue = Desc.ClearValue;
        }

        auto InitialState = Subresources.empty() ? D3D12_RESOURCE_STATE_COMMON : D3D12_RESOURCE_STATE_COPY_DEST;

        auto Allocator = Dx12RenderDevice::Get()->GetAllocator()->GetMA();
        ThrowIfFailed(Allocator->CreateResource(
            &AllocDesc,
            &Dx12Desc,
            InitialState,
            ClearValuePtr,
            &m_Allocation,
            IID_PPV_ARGS(&m_Resource)));

        InitializeDesc();

        Dx12ResourceStateManager::Get()->StartTrakingResource(m_Resource.Get(), InitialState);
        if (!Subresources.empty())
        {
            CopyFrom(0, Subresources, *CopyId);
        }
    }

    Dx12Texture::Dx12Texture(
        WinAPI::ComPtr<ID3D12Resource>      Texture,
        D3D12_RESOURCE_STATES               InitialState,
        WinAPI::ComPtr<D3D12MA::Allocation> Allocation)
    {
        m_Resource   = std::move(Texture);
        m_Allocation = std::move(Allocation);

        if (m_Resource)
        {
            InitializeDesc();
            Dx12ResourceStateManager::Get()->StartTrakingResource(m_Resource.Get(), InitialState);
        }
    }

    Dx12Texture::Dx12Texture(
        WinAPI::ComPtr<ID3D12Resource>      Texture,
        WinAPI::ComPtr<D3D12MA::Allocation> Allocation,
        std::span<const SubresourceDesc>    Subresources,
        uint64_t&                           CopyId) :
        Dx12Texture{ std::move(Texture), D3D12_RESOURCE_STATE_COPY_DEST, std::move(Allocation) }
    {
        CopyFrom(0, Subresources, CopyId);
    }

    Dx12Texture::~Dx12Texture()
    {
        if (m_Resource)
        {
            Dx12ResourceStateManager::Get()->StopTrakingResource(m_Resource.Get());
            Dx12Swapchain::Get()->SafeRelease(m_Resource, m_Allocation);
        }
    }

    Vector3I Dx12Texture::GetDimensions() const
    {
        return {
            int(m_Desc.Width),
            int(m_Desc.Height),
            int(m_Desc.Depth)
        };
    }

    uint16_t Dx12Texture::GetMipLevels() const
    {
        return m_Desc.MipLevels;
    }

    uint32_t Dx12Texture::GetSubResourceCount() const
    {
        return uint32_t(m_Desc.MipLevels) * m_Desc.Depth;
    }

    uint32_t Dx12Texture::GetSubresourceIndex(
        uint32_t PlaneIndex,
        uint32_t ArrayIndex,
        uint32_t MipIndex) const
    {
        return MipIndex +
               ArrayIndex * m_Desc.MipLevels +
               PlaneIndex * m_Desc.Depth * m_Desc.MipLevels;
    }

    void Dx12Texture::InitializeDesc()
    {
        NEON_ASSERT(m_Resource);
        auto Dx12Desc = m_Resource->GetDesc();

        m_Desc = {
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
            m_Desc.Layout = ResourceLayout::Unknown;
            break;
        case D3D12_TEXTURE_LAYOUT_ROW_MAJOR:
            m_Desc.Layout = ResourceLayout::RowMajor;
            break;
        case D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE:
            m_Desc.Layout = ResourceLayout::StandardSwizzle64KB;
            break;
        case D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE:
            m_Desc.Layout = ResourceLayout::UndefinedSwizzle64KB;
            break;
        default:
            break;
        }

        switch (Dx12Desc.Dimension)
        {
        case D3D12_RESOURCE_DIMENSION_BUFFER:
            m_Desc.Type = ResourceType::Buffer;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            m_Desc.Type = ResourceType::Texture1D;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            m_Desc.Type = ResourceType::Texture2D;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            m_Desc.Type = ResourceType::Texture3D;
            break;
        }
    }

    size_t Dx12Texture::GetTextureCopySize(
        uint32_t SubresourcesCount)
    {
        size_t TotalBytes;
        auto   Desc = m_Resource->GetDesc();

        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        Dx12Device->GetCopyableFootprints(
            &Desc,
            0,
            SubresourcesCount,
            0,
            nullptr,
            nullptr,
            nullptr,
            &TotalBytes);

        return TotalBytes;
    }

    const Ptr<ITexture>& ITexture::GetDefault(
        DefaultTextures Type)
    {
        return Dx12RenderDevice::Get()->GetDefaultTexture(Type);
    }
} // namespace Neon::RHI