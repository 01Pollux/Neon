#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/Resource/Descriptor.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    static uint32_t LimitDescriptorHeapSize(
        bool     Sampler,
        uint32_t Count)
    {
        auto& Features = Dx12RenderDevice::Get()->GetFeatures();
        return std::min<uint32_t>(Count, Features.MaxDescriptorHeapSize(Sampler));
    }

    DescriptorType CastDescriptorType(
        D3D12_DESCRIPTOR_HEAP_TYPE Type)
    {
        switch (Type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return DescriptorType::ResourceView;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            return DescriptorType::RenderTargetView;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            return DescriptorType::DepthStencilView;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return DescriptorType::Sampler;
        default:
            std::unreachable();
        }
    }

    D3D12_DESCRIPTOR_HEAP_TYPE CastDescriptorType(
        DescriptorType Type)
    {
        switch (Type)
        {
        case DescriptorType::ResourceView:
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        case DescriptorType::RenderTargetView:
            return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        case DescriptorType::DepthStencilView:
            return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        case DescriptorType::Sampler:
            return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        default:
            std::unreachable();
        }
    }

    //

    IDescriptorHeap* IDescriptorHeap::Create(
        DescriptorType Type,
        uint32_t       MaxCount,
        bool           ShaderVisible)
    {
        return NEON_NEW Dx12DescriptorHeap(
            CastDescriptorType(Type),
            LimitDescriptorHeapSize(Type == DescriptorType::Sampler, MaxCount),
            ShaderVisible);
    }

    //

    Dx12DescriptorHeap::Dx12DescriptorHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE Type,
        uint32_t                   MaxCount,
        bool                       ShaderVisible) :
        m_HeapSize(MaxCount),
        m_HeapType(Type)
    {
        D3D12_DESCRIPTOR_HEAP_DESC Desc{
            .Type           = Type,
            .NumDescriptors = uint32_t(MaxCount),
            .Flags          = ShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask       = 1
        };

        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateDescriptorHeap(
            &Desc,
            IID_PPV_ARGS(&m_DescriptorHeap)));

        m_CpuHandle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        if (ShaderVisible)
        {
            m_GpuHandle = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        }
    }

    Dx12DescriptorHeap::~Dx12DescriptorHeap()
    {
        if (m_DescriptorHeap)
        {
            Dx12Swapchain::Get()->SafeRelease(m_DescriptorHeap);
            m_DescriptorHeap = nullptr;
        }
    }

    void Dx12DescriptorHeap::Copy(
        uint32_t        DescriptorIndex,
        const CopyInfo& SrcDescriptors)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        Dx12Device->CopyDescriptorsSimple(
            uint32_t(SrcDescriptors.CopySize),
            { GetCPUAddress(DescriptorIndex).Value },
            { SrcDescriptors.Descriptor.Value },
            m_HeapType);
    }

    void IDescriptorHeap::Copy(
        DescriptorType            DescriptorType,
        std::span<const CopyInfo> SrcDescriptors,
        std::span<const CopyInfo> DstDescriptors)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        auto DxDescriptors(std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(DstDescriptors.size()));
        auto DxDescriptorsSizes(std::make_unique<UINT[]>(DstDescriptors.size()));

        auto DxSrcDescriptors(std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(SrcDescriptors.size()));
        auto DxSrcDescriptorsSizes(std::make_unique<UINT[]>(SrcDescriptors.size()));

        using DesciptorTuple = std::tuple<
            D3D12_CPU_DESCRIPTOR_HANDLE*,
            UINT*,
            std::span<const CopyInfo>*>;

        for (auto& [DxDesc, DxSize, Info] : {
                 DesciptorTuple{ DxDescriptors.get(), DxDescriptorsSizes.get(), &DstDescriptors },
                 DesciptorTuple{ DxSrcDescriptors.get(), DxSrcDescriptorsSizes.get(), &SrcDescriptors } })
        {
            for (uint32_t i = 0; i < Info->size(); i++)
            {
                DxDesc[i] = { (*Info)[i].Descriptor.Value };
                DxSize[i] = uint32_t((*Info)[i].CopySize);
            }
        }

        Dx12Device->CopyDescriptors(
            UINT(DstDescriptors.size()),
            DxDescriptors.get(),
            DxDescriptorsSizes.get(),
            UINT(SrcDescriptors.size()),
            DxSrcDescriptors.get(),
            DxSrcDescriptorsSizes.get(),
            CastDescriptorType(DescriptorType));
    }

    CpuDescriptorHandle Dx12DescriptorHeap::GetCPUAddress(
        uint32_t Offset)
    {
        auto RenderDevice = Dx12RenderDevice::Get();

        CpuDescriptorHandle Address(m_CpuHandle.ptr);
        Address.Value += Offset * RenderDevice->GetDescriptorSize(m_HeapType);
        return Address;
    }

    GpuDescriptorHandle Dx12DescriptorHeap::GetGPUAddress(
        uint32_t Offset)
    {
        auto RenderDevice = Dx12RenderDevice::Get();

        GpuDescriptorHandle Address(m_GpuHandle.ptr);
        Address.Value += Offset * RenderDevice->GetDescriptorSize(m_HeapType);
        return Address;
    }

    bool Dx12DescriptorHeap::IsDescriptorInRange(
        CpuDescriptorHandle Handle)
    {
        return (m_CpuHandle.ptr >= Handle.Value) && ((m_CpuHandle.ptr + m_HeapSize) < Handle.Value);
    }

    bool Dx12DescriptorHeap::IsDescriptorInRange(
        GpuDescriptorHandle Handle)
    {
        return (m_GpuHandle.ptr >= Handle.Value) && ((m_GpuHandle.ptr + m_HeapSize) < Handle.Value);
    }

    //

    void Dx12DescriptorHeap::CreateConstantBufferView(
        uint32_t       DescriptorIndex,
        const CBVDesc& Desc)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        D3D12_CONSTANT_BUFFER_VIEW_DESC Dx12Desc(Desc.Resource.Value, uint32_t(Desc.Size));
        Dx12Device->CreateConstantBufferView(
            &Dx12Desc,
            { GetCPUAddress(DescriptorIndex).Value });
    }

    void Dx12DescriptorHeap::CreateShaderResourceView(
        uint32_t       DescriptorIndex,
        IGpuResource*  Resource,
        const SRVDesc* Desc)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        D3D12_SHADER_RESOURCE_VIEW_DESC  Dx12Desc;
        D3D12_SHADER_RESOURCE_VIEW_DESC* Dx12DescPtr = nullptr;

        if (Desc)
        {
            Dx12DescPtr = &Dx12Desc;

            std::visit(
                VariantVisitor{
                    [&Dx12Desc](const SRVDesc::Buffer& Buffer)
                    {
                        Dx12Desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                        Dx12Desc.Buffer        = {
                                   .FirstElement        = Buffer.FirstElement,
                                   .NumElements         = Buffer.Count,
                                   .StructureByteStride = Buffer.SizeOfStruct,
                                   .Flags               = Buffer.Raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE
                        };
                    },
                    [&Dx12Desc](const SRVDesc::Texture1D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                        Dx12Desc.Texture1D     = {
                                .MostDetailedMip     = Texture.MostDetailedMip,
                                .MipLevels           = Texture.MipLevels,
                                .ResourceMinLODClamp = Texture.MinLODClamp
                        };
                    },
                    [&Dx12Desc](const SRVDesc::Texture1DArray& Texture)
                    {
                        Dx12Desc.ViewDimension  = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                        Dx12Desc.Texture1DArray = {
                            .MostDetailedMip     = Texture.MostDetailedMip,
                            .MipLevels           = Texture.MipLevels,
                            .FirstArraySlice     = Texture.StartSlice,
                            .ArraySize           = Texture.Size,
                            .ResourceMinLODClamp = Texture.MinLODClamp
                        };
                    },
                    [&Dx12Desc](const SRVDesc::Texture2D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                        Dx12Desc.Texture2D     = {
                                .MostDetailedMip     = Texture.MostDetailedMip,
                                .MipLevels           = Texture.MipLevels,
                                .PlaneSlice          = Texture.PlaneSlice,
                                .ResourceMinLODClamp = Texture.MinLODClamp
                        };
                    },
                    [&Dx12Desc](const SRVDesc::Texture2DArray& Texture)
                    {
                        Dx12Desc.ViewDimension  = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        Dx12Desc.Texture2DArray = {
                            .MostDetailedMip     = Texture.MostDetailedMip,
                            .MipLevels           = Texture.MipLevels,
                            .FirstArraySlice     = Texture.StartSlice,
                            .ArraySize           = Texture.Size,
                            .PlaneSlice          = Texture.PlaneSlice,
                            .ResourceMinLODClamp = Texture.MinLODClamp
                        };
                    },
                    [&Dx12Desc](const SRVDesc::Texture2DMS& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                        Dx12Desc.Texture2DMS   = {};
                    },
                    [&Dx12Desc](const SRVDesc::Texture2DMSArray& Texture)
                    {
                        Dx12Desc.ViewDimension  = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        Dx12Desc.Texture2DArray = {
                            .FirstArraySlice = Texture.StartSlice,
                            .ArraySize       = Texture.Size
                        };
                    },
                    [&Dx12Desc](const SRVDesc::Texture3D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                        Dx12Desc.Texture3D     = {
                                .MostDetailedMip     = Texture.MostDetailedMip,
                                .MipLevels           = Texture.MipLevels,
                                .ResourceMinLODClamp = Texture.MinLODClamp
                        };
                    },
                    [&Dx12Desc](const SRVDesc::TextureCube& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                        Dx12Desc.TextureCube   = {
                              .MostDetailedMip     = Texture.MostDetailedMip,
                              .MipLevels           = Texture.MipLevels,
                              .ResourceMinLODClamp = Texture.MinLODClamp
                        };
                    },
                    [&Dx12Desc](const SRVDesc::TextureCubeArray& Texture)
                    {
                        Dx12Desc.ViewDimension    = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                        Dx12Desc.TextureCubeArray = {
                            .MostDetailedMip     = Texture.MostDetailedMip,
                            .MipLevels           = Texture.MipLevels,
                            .First2DArrayFace    = Texture.StartSlice,
                            .NumCubes            = Texture.Size,
                            .ResourceMinLODClamp = Texture.MinLODClamp
                        };
                    },
                    [&Dx12Desc](const SRVDesc::RaytracingAccelerationStructure& AccelStruct)
                    {
                        Dx12Desc.ViewDimension                   = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
                        Dx12Desc.RaytracingAccelerationStructure = {
                            .Location = AccelStruct.Resource.Value
                        };
                    } },
                Desc->View);

            Dx12Desc.Format                  = CastFormat(Desc->Format);
            Dx12Desc.Shader4ComponentMapping = Desc->Mapping;
        }

        Dx12Device->CreateShaderResourceView(
            GetDx12Resource(Resource),
            Dx12DescPtr,
            { GetCPUAddress(DescriptorIndex).Value });
    }

    void Dx12DescriptorHeap::CreateUnorderedAccessView(
        uint32_t       DescriptorIndex,
        IGpuResource*  Resource,
        const UAVDesc* Desc,
        IGpuResource*  CounterBuffer)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        D3D12_UNORDERED_ACCESS_VIEW_DESC  Dx12Desc;
        D3D12_UNORDERED_ACCESS_VIEW_DESC* Dx12DescPtr = nullptr;

        if (Desc)
        {
            Dx12DescPtr = &Dx12Desc;
            std::visit(
                VariantVisitor{
                    [&Dx12Desc](const UAVDesc::Buffer& Buffer)
                    {
                        Dx12Desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                        Dx12Desc.Buffer        = {
                                   .FirstElement         = Buffer.FirstElement,
                                   .NumElements          = Buffer.Count,
                                   .StructureByteStride  = Buffer.SizeOfStruct,
                                   .CounterOffsetInBytes = Buffer.CounterOffset,
                                   .Flags                = Buffer.Raw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE

                        };
                    },
                    [&Dx12Desc](const UAVDesc::Texture1D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                        Dx12Desc.Texture1D     = {
                                .MipSlice = Texture.MipSlice
                        };
                    },
                    [&Dx12Desc](const UAVDesc::Texture1DArray& Texture)
                    {
                        Dx12Desc.ViewDimension  = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                        Dx12Desc.Texture1DArray = {
                            .MipSlice        = Texture.MipSlice,
                            .FirstArraySlice = Texture.StartSlice,
                            .ArraySize       = Texture.Size
                        };
                    },
                    [&Dx12Desc](const UAVDesc::Texture2D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                        Dx12Desc.Texture2D     = {
                                .MipSlice   = Texture.MipSlice,
                                .PlaneSlice = Texture.PlaneSlice
                        };
                    },
                    [&Dx12Desc](const UAVDesc::Texture2DArray& Texture)
                    {
                        Dx12Desc.ViewDimension  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                        Dx12Desc.Texture2DArray = {
                            .MipSlice        = Texture.MipSlice,
                            .FirstArraySlice = Texture.StartSlice,
                            .ArraySize       = Texture.Size,
                            .PlaneSlice      = Texture.PlaneSlice
                        };
                    },
                    [&Dx12Desc](const UAVDesc::Texture3D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                        Dx12Desc.Texture3D     = {
                                .MipSlice    = Texture.MipSlice,
                                .FirstWSlice = Texture.FirstWSlice,
                                .WSize       = Texture.Size
                        };
                    } },
                Desc->View);

            Dx12Desc.Format = CastFormat(Desc->Format);
        }

        Dx12Device->CreateUnorderedAccessView(
            GetDx12Resource(Resource),
            CounterBuffer ? GetDx12Resource(CounterBuffer) : nullptr,
            Dx12DescPtr,
            { GetCPUAddress(DescriptorIndex).Value });
    }

    void Dx12DescriptorHeap::CreateRenderTargetView(
        uint32_t       DescriptorIndex,
        IGpuResource*  Resource,
        const RTVDesc* Desc)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        D3D12_RENDER_TARGET_VIEW_DESC  Dx12Desc;
        D3D12_RENDER_TARGET_VIEW_DESC* Dx12DescPtr = nullptr;

        if (Desc)
        {
            Dx12DescPtr = &Dx12Desc;
            std::visit(
                VariantVisitor{
                    [&Dx12Desc](const RTVDesc::Buffer& Buffer)
                    {
                        Dx12Desc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
                        Dx12Desc.Buffer        = {
                                   .FirstElement = Buffer.FirstElement,
                                   .NumElements  = Buffer.Count
                        };
                    },
                    [&Dx12Desc](const RTVDesc::Texture1D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                        Dx12Desc.Texture1D     = {
                                .MipSlice = Texture.MipSlice
                        };
                    },
                    [&Dx12Desc](const RTVDesc::Texture1DArray& Texture)
                    {
                        Dx12Desc.ViewDimension  = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                        Dx12Desc.Texture1DArray = {
                            .MipSlice        = Texture.MipSlice,
                            .FirstArraySlice = Texture.StartSlice,
                            .ArraySize       = Texture.Size
                        };
                    },
                    [&Dx12Desc](const RTVDesc::Texture2D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                        Dx12Desc.Texture2D     = {
                                .MipSlice   = Texture.MipSlice,
                                .PlaneSlice = Texture.PlaneSlice
                        };
                    },
                    [&Dx12Desc](const RTVDesc::Texture2DArray& Texture)
                    {
                        Dx12Desc.ViewDimension  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                        Dx12Desc.Texture2DArray = {
                            .MipSlice        = Texture.MipSlice,
                            .FirstArraySlice = Texture.StartSlice,
                            .ArraySize       = Texture.Size,
                            .PlaneSlice      = Texture.PlaneSlice
                        };
                    },
                    [&Dx12Desc](const RTVDesc::Texture2DMS& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    },
                    [&Dx12Desc](const RTVDesc::Texture2DMSArray& Texture)
                    {
                        Dx12Desc.ViewDimension    = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                        Dx12Desc.Texture2DMSArray = {
                            .FirstArraySlice = Texture.StartSlice,
                            .ArraySize       = Texture.Size
                        };
                    },
                    [&Dx12Desc](const RTVDesc::Texture3D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                        Dx12Desc.Texture3D     = {
                                .MipSlice    = Texture.MipSlice,
                                .FirstWSlice = Texture.FirstWSlice,
                                .WSize       = Texture.WSize
                        };
                    } },
                Desc->View);

            Dx12Desc.Format = CastFormat(Desc->Format);
        }

        Dx12Device->CreateRenderTargetView(
            GetDx12Resource(Resource),
            Dx12DescPtr,
            { GetCPUAddress(DescriptorIndex).Value });
    }

    void Dx12DescriptorHeap::CreateDepthStencilView(
        uint32_t       DescriptorIndex,
        IGpuResource*  Resource,
        const DSVDesc* Desc)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        D3D12_DEPTH_STENCIL_VIEW_DESC  Dx12Desc;
        D3D12_DEPTH_STENCIL_VIEW_DESC* Dx12DescPtr = nullptr;

        if (Desc)
        {
            Dx12DescPtr = &Dx12Desc;
            std::visit(
                VariantVisitor{
                    [&Dx12Desc](const DSVDesc::Texture1D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                        Dx12Desc.Texture1D     = {
                                .MipSlice = Texture.MipSlice
                        };
                    },
                    [&Dx12Desc](const DSVDesc::Texture1DArray& Texture)
                    {
                        Dx12Desc.ViewDimension  = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                        Dx12Desc.Texture1DArray = {
                            .MipSlice        = Texture.MipSlice,
                            .FirstArraySlice = Texture.StartSlice,
                            .ArraySize       = Texture.Size
                        };
                    },
                    [&Dx12Desc](const DSVDesc::Texture2D& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                        Dx12Desc.Texture2D     = {
                                .MipSlice = Texture.MipSlice
                        };
                    },
                    [&Dx12Desc](const DSVDesc::Texture2DArray& Texture)
                    {
                        Dx12Desc.ViewDimension  = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                        Dx12Desc.Texture2DArray = {
                            .MipSlice        = Texture.MipSlice,
                            .FirstArraySlice = Texture.StartSlice,
                            .ArraySize       = Texture.Size
                        };
                    },
                    [&Dx12Desc](const DSVDesc::Texture2DMS& Texture)
                    {
                        Dx12Desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    },
                    [&Dx12Desc](const DSVDesc::Texture2DMSArray& Texture)
                    {
                        Dx12Desc.ViewDimension    = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                        Dx12Desc.Texture2DMSArray = {
                            .FirstArraySlice = Texture.StartSlice,
                            .ArraySize       = Texture.Size
                        };
                    } },
                Desc->View);

            if (Desc->ReadOnlyDepth)
            {
                Dx12Desc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
            }
            if (Desc->ReadOnlyStencil)
            {
                Dx12Desc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
            }
            Dx12Desc.Format = CastFormat(Desc->Format);
        }

        Dx12Device->CreateDepthStencilView(
            GetDx12Resource(Resource),
            Dx12DescPtr,
            { GetCPUAddress(DescriptorIndex).Value });
    }

    void Dx12DescriptorHeap::CreateSampler(
        uint32_t           DescriptorIndex,
        const SamplerDesc& Desc)
    {
        auto               Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        D3D12_SAMPLER_DESC Dx12Desc   = {
              .Filter         = CastFilter(Desc.Filter),
              .AddressU       = CastAddressMode(Desc.AddressU),
              .AddressV       = CastAddressMode(Desc.AddressV),
              .AddressW       = CastAddressMode(Desc.AddressW),
              .MipLODBias     = Desc.MipLODBias,
              .MaxAnisotropy  = Desc.MaxAnisotropy,
              .ComparisonFunc = CastComparisonFunc(Desc.ComparisonFunc),
              .BorderColor    = { Desc.BorderColor[0], Desc.BorderColor[1], Desc.BorderColor[2], Desc.BorderColor[3] },
              .MinLOD         = Desc.MinLOD,
              .MaxLOD         = Desc.MaxLOD
        };
        Dx12Device->CreateSampler(
            &Dx12Desc,
            { GetCPUAddress(DescriptorIndex).Value });
    }

    //

    ID3D12DescriptorHeap* Dx12DescriptorHeap::Get() const noexcept
    {
        return m_DescriptorHeap.Get();
    }

    uint32_t Dx12DescriptorHeap::GetSize() const noexcept
    {
        return m_HeapSize;
    }

    D3D12_DESCRIPTOR_HEAP_TYPE Dx12DescriptorHeap::GetType() const noexcept
    {
        return m_HeapType;
    }

    //

    IDescriptorHeapAllocator* IDescriptorHeapAllocator::Create(
        AllocationType Type,
        DescriptorType DescType,
        uint32_t       SizeOfHeap,
        bool           ShaderVisible)
    {
        switch (Type)
        {
        case AllocationType::Ring:
            return NEON_NEW Dx12RingDescriptorHeapAllocator(CastDescriptorType(DescType), SizeOfHeap, ShaderVisible);
        case AllocationType::Buddy:
            return NEON_NEW Dx12DescriptorHeapBuddyAllocator(CastDescriptorType(DescType), SizeOfHeap, ShaderVisible);
        default:
            std::unreachable();
        }
    }

    //

    Dx12RingDescriptorHeapAllocator::Dx12RingDescriptorHeapAllocator(
        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
        uint32_t                   MaxCount,
        bool                       ShaderVisible) :
        m_HeapDescriptor(DescriptorType, MaxCount, ShaderVisible)
    {
    }

    DescriptorHeapHandle Dx12RingDescriptorHeapAllocator::Allocate(
        uint32_t DescriptorSize)
    {
        std::lock_guard DescLock(m_DescriptorLock);

        uint32_t HeapOffset = m_CurrentDescriptorOffset;
        m_CurrentDescriptorOffset += DescriptorSize;
        if (m_CurrentDescriptorOffset >= m_HeapDescriptor.GetSize())
        {
            m_CurrentDescriptorOffset %= m_HeapDescriptor.GetSize();
            HeapOffset = 0;
        }

        return {
            .Heap   = &m_HeapDescriptor,
            .Offset = HeapOffset,
            .Size   = DescriptorSize
        };
    }

    void Dx12RingDescriptorHeapAllocator::Free(
        std::span<DescriptorHeapHandle>)
    {
    }

    void Dx12RingDescriptorHeapAllocator::FreeAll()
    {
    }

    IDescriptorHeap* Dx12RingDescriptorHeapAllocator::GetHeap(
        uint32_t)
    {
        return &m_HeapDescriptor;
    }

    //

    Dx12DescriptorHeapBuddyAllocator::BuddyBlock::BuddyBlock(
        const HeapDescriptorAllocInfo& Info) :
        Heap(Info.DescriptorType, Info.SizeOfHeap, Info.ShaderVisible),
        Allocator(Info.SizeOfHeap)
    {
    }

    Dx12DescriptorHeapBuddyAllocator::Dx12DescriptorHeapBuddyAllocator(
        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
        uint32_t                   SizeOfHeap,
        bool                       ShaderVisible) :
        m_HeapBlockAllocInfo{ SizeOfHeap, DescriptorType, ShaderVisible }
    {
    }

    DescriptorHeapHandle Dx12DescriptorHeapBuddyAllocator::Allocate(
        uint32_t DescriptorSize)
    {
        std::lock_guard HeapLock(m_HeapsBlockMutex);

        bool Allocated = false;
        for (auto Iter = m_HeapBlocks.begin(); Iter != m_HeapBlocks.end(); Iter++)
        {
            if (auto Hndl = Iter->Allocator.Allocate(DescriptorSize))
            {
                return {
                    .Heap   = &Iter->Heap,
                    .Offset = uint32_t(Hndl.Offset),
                    .Size   = uint32_t(Hndl.Size)
                };
            }
        }

        // Grow the heap for each new allocation
        if (m_HeapBlocks.empty()) [[likely]]
        {
            m_HeapBlockAllocInfo.SizeOfHeap *= 2;
        }

        while (m_HeapBlockAllocInfo.SizeOfHeap < DescriptorSize)
        {
            m_HeapBlockAllocInfo.SizeOfHeap *= 2;
        }
        m_HeapBlockAllocInfo.SizeOfHeap = LimitDescriptorHeapSize(m_HeapBlockAllocInfo.DescriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, m_HeapBlockAllocInfo.SizeOfHeap);

        auto& Block = m_HeapBlocks.emplace_back(m_HeapBlockAllocInfo);
        auto  Hndl  = Block.Allocator.Allocate(DescriptorSize);

        return {
            .Heap   = &Block.Heap,
            .Offset = uint32_t(Hndl.Offset),
            .Size   = uint32_t(Hndl.Size)
        };
    }

    void Dx12DescriptorHeapBuddyAllocator::Free(
        std::span<DescriptorHeapHandle> Handles)
    {
        std::lock_guard HeapLock(m_HeapsBlockMutex);

        for (auto& Data : Handles)
        {
            bool Exists = false;
            for (auto& Block : m_HeapBlocks)
            {
                if (Data.Heap == &Block.Heap)
                {
                    Block.Allocator.Free({ .Offset = Data.Offset, .Size = Data.Size });
                    Exists = true;
                    break;
                }
            }
            NEON_ASSERT(Exists, "Tried to free a non-existant heap");
        }
    }

    void Dx12DescriptorHeapBuddyAllocator::FreeAll()
    {
        std::lock_guard HeapLock(m_HeapsBlockMutex);

        for (auto& Block : m_HeapBlocks)
        {
            Block.Allocator = Allocator::BuddyAllocator(Block.Heap.GetSize());
        }
    }

    IDescriptorHeap* Dx12DescriptorHeapBuddyAllocator::GetHeap(
        uint32_t Index)
    {
        NEON_ASSERT(Index <= m_HeapBlocks.size());
        return &std::next(m_HeapBlocks.begin(), Index)->Heap;
    }
} // namespace Neon::RHI