#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/Resource/Descriptor.hpp>
#include <Private/RHI/Dx12/Device.hpp>

namespace Neon::RHI
{
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
        size_t         MaxCount,
        bool           ShaderVisible)
    {
        return NEON_NEW Dx12DescriptorHeap(
            CastDescriptorType(Type),
            MaxCount,
            ShaderVisible);
    }

    //

    Dx12DescriptorHeap::Dx12DescriptorHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE Type,
        size_t                     MaxCount,
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
            IID_PPV_ARGS(&m_HeapDescriptor)));

        m_CpuHandle = m_HeapDescriptor->GetCPUDescriptorHandleForHeapStart();
        if (ShaderVisible)
        {
            m_GpuHandle = m_HeapDescriptor->GetGPUDescriptorHandleForHeapStart();
        }
    }

    void Dx12DescriptorHeap::Copy(
        size_t          DescriptorIndex,
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
            for (size_t i = 0; i < Info->size(); i++)
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
        size_t Offset)
    {
        auto RenderDevice = Dx12RenderDevice::Get();

        CpuDescriptorHandle Address(m_CpuHandle.ptr);
        Address.Value += Offset * RenderDevice->GetDescriptorSize(m_HeapType);
        return Address;
    }

    GpuDescriptorHandle Dx12DescriptorHeap::GetGPUAddress(
        size_t Offset)
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
        size_t         DescriptorIndex,
        const CBVDesc& Desc)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        D3D12_CONSTANT_BUFFER_VIEW_DESC Dx12Desc(Desc.Resource.Value, uint32_t(Desc.Size));
        Dx12Device->CreateConstantBufferView(
            &Dx12Desc,
            { GetCPUAddress(DescriptorIndex).Value });
    }

    void Dx12DescriptorHeap::CreateShaderResourceView(
        size_t         DescriptorIndex,
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
        }

        Dx12Desc.Format                  = CastFormat(Desc->Format);
        Dx12Desc.Shader4ComponentMapping = Desc->Mapping;

        Dx12Device->CreateShaderResourceView(
            GetDx12Resource(Resource),
            Dx12DescPtr,
            { GetCPUAddress(DescriptorIndex).Value });
    }

    void Dx12DescriptorHeap::CreateUnorderedAccessView(
        size_t         DescriptorIndex,
        IGpuResource*  Resource,
        const UAVDesc* Desc,
        IGpuResource*  CounterBuffer)
    {
    }

    void Dx12DescriptorHeap::CreateRenderTargetView(
        size_t         DescriptorIndex,
        IGpuResource*  Resource,
        const RTVDesc* Desc)
    {
    }

    void Dx12DescriptorHeap::CreateDepthStencilView(
        size_t         DescriptorIndex,
        IGpuResource*  Resource,
        const DSVDesc* Desc)
    {
    }

    void Dx12DescriptorHeap::CreateSampler(
        size_t             DescriptorIndex,
        const SamplerDesc& Desc)
    {
    }

    //

    ID3D12DescriptorHeap* Dx12DescriptorHeap::Get() const noexcept
    {
        return nullptr;
    }

    //

    IDescriptorHeapAllocator* IDescriptorHeapAllocator::Create(
        AllocationType Type,
        DescriptorType DescType,
        size_t         SizeOfHeap,
        bool           ShaderVisible)
    {
        return nullptr;
    }

    //

    RingDescriptorHeapAllocator::RingDescriptorHeapAllocator(
        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
        size_t                     MaxCount,
        bool                       ShaderVisible)
    {
    }

    DescriptorHeapHandle RingDescriptorHeapAllocator::Allocate(
        size_t DescriptorSize)
    {
        return DescriptorHeapHandle();
    }

    void RingDescriptorHeapAllocator::Free(
        const DescriptorHeapHandle& Data)
    {
    }

    void RingDescriptorHeapAllocator::FreeAll()
    {
    }

    //

    DescriptorHeapBuddyAllocator::BuddyBlock::BuddyBlock(
        const HeapDescriptorAllocInfo& Info)
    {
    }

    DescriptorHeapBuddyAllocator::DescriptorHeapBuddyAllocator(
        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
        size_t                     SizeOfHeap,
        bool                       ShaderVisible)
    {
    }

    DescriptorHeapHandle DescriptorHeapBuddyAllocator::Allocate(
        size_t DescriptorSize)
    {
        return DescriptorHeapHandle();
    }

    void DescriptorHeapBuddyAllocator::Free(
        const DescriptorHeapHandle& Data)
    {
    }

    void DescriptorHeapBuddyAllocator::FreeAll()
    {
    }

    IDescriptorHeap* DescriptorHeapBuddyAllocator::GetHeap(
        size_t Index)
    {
        return nullptr;
    }
} // namespace Neon::RHI