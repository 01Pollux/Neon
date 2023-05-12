#include <EnginePCH.hpp>
#include <Renderer/RenderGraph/Storage.hpp>

#include <Math/Size2.hpp>
#include <RHI/DisplayBuffers.hpp>

namespace Renderer::RG
{
    RenderGraphStorage::RenderGraphStorage(
        RHI::IDisplayBuffers* Display) :
        m_Display(Display)
    {
    }

    void RenderGraphStorage::Reset()
    {
        FlushResources();
        m_Resources.clear();
        m_RootSignatures.clear();
        m_PipelineStates.clear();
    }

    bool RenderGraphStorage::ContainsResource(
        const ResourceId& Id) const
    {
        return m_Resources.contains(Id);
    }

    bool RenderGraphStorage::ContainsResourceView(
        const ResourceViewId& ViewId) const
    {
        bool Found = false;
        auto Iter  = m_Resources.find(ViewId.GetResource());
        if (Iter != m_Resources.end())
        {
            auto& Views = Iter->second.GetViews();
            Found       = Views.contains(ViewId.Get());
        }
        return Found;
    }

    //

    ResourceHandle& RenderGraphStorage::GetResource(
        const ResourceId& Id)
    {
        auto Iter = m_Resources.find(Id);
        NEON_ASSERT_MSG(Iter != m_Resources.end(), "Resource doesn't exists");
        return Iter->second;
    }

    const ResourceHandle& RenderGraphStorage::GetResource(
        const ResourceId& Id) const
    {
        auto Iter = m_Resources.find(Id);
        NEON_ASSERT_MSG(Iter != m_Resources.end(), "Resource doesn't exists");
        return Iter->second;
    }

    const ResourceViewDesc& RenderGraphStorage::GetResourceView(
        const ResourceViewId&        ViewId,
        D3D12_CPU_DESCRIPTOR_HANDLE* CpuHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE* GpuHandle) const
    {
        auto Iter = m_Resources.find(ViewId.GetResource());
        if (Iter != m_Resources.end())
        {
            auto& Views    = Iter->second.GetViews();
            auto  ViewIter = Views.find(ViewId.Get());

            if (ViewIter != Views.end())
            {
                auto& ViewInfo = ViewIter->second;
                if (CpuHandle)
                {
                    *CpuHandle = ViewInfo.first.Heap->GetCPUAddress(ViewInfo.first.Offset);
                }
                if (GpuHandle)
                {
                    *GpuHandle = ViewInfo.first.Heap->GetGPUAddress(ViewInfo.first.Offset);
                }
                return ViewInfo.second;
            }
        }

        NEON_ASSERT_MSG(false, "Resource view doesn't exists");
        std::unreachable();
    }

    RHI::IDisplayBuffers* RenderGraphStorage::GetDisplay() const
    {
        return m_Display;
    }

    //

    const RHI::RootSignature& RenderGraphStorage::GetRootSignature(
        const ResourceId& Id) const
    {
        auto Iter = m_RootSignatures.find(Id);
        NEON_ASSERT_MSG(Iter != m_RootSignatures.end(), "Root signature doesn't exists");
        return Iter->second;
    }

    const RHI::PipelineState& RenderGraphStorage::GetPipelineState(
        const ResourceId& Id) const
    {
        auto Iter = m_PipelineStates.find(Id);
        NEON_ASSERT_MSG(Iter != m_PipelineStates.end(), "Pipeline state doesn't exists");
        return Iter->second;
    }

    //

    void RenderGraphStorage::DeclareBuffer(
        const ResourceId&       Id,
        const ResourceDesc&     Desc,
        RHI::GraphicsBufferType Type)
    {
        NEON_ASSERT(Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER);
        auto HandleIter = m_Resources.emplace(Id, ResourceHandle(Id, Desc, ResourceFlags::None, Type));
        NEON_ASSERT_MSG(HandleIter.second, "Resource already exists");
    }

    void RenderGraphStorage::DeclareTexture(
        const ResourceId&   Id,
        const ResourceDesc& Desc,
        ResourceFlags       Flags)
    {
        NEON_ASSERT(Desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER);

        auto HandleIter = m_Resources.emplace(Id, ResourceHandle(Id, Desc, Flags));
        NEON_ASSERT_MSG(HandleIter.second, "Resource already exists");
    }

    void RenderGraphStorage::ImportBuffer(
        const ResourceId&                 Id,
        const RHI::GraphicsResource::Ptr& Resource,
        RHI::GraphicsBufferType           BufferType)
    {
        NEON_ASSERT_MSG(!ContainsResource(Id), "Resource already exists");
        auto HandleIter = m_Resources.emplace(Id, ResourceHandle(Id, Resource, BufferType)).first;
        RHI::RenameObject(HandleIter->second.Get()->Get(), Id.GetName());

        m_ImportedResources.emplace(Id);
    }

    void RenderGraphStorage::ImportTexture(
        const ResourceId&                 Id,
        const RHI::GraphicsResource::Ptr& Resource,
        const ClearOperationOpt&          ClearValue)
    {
        NEON_ASSERT_MSG(!ContainsResource(Id), "Resource already exists");
        auto HandleIter = m_Resources.emplace(Id, ResourceHandle(Id, Resource, ClearValue)).first;
        RHI::RenameObject(HandleIter->second.Get()->Get(), Id.GetName());

        auto& Handle = HandleIter->second;
        if (ClearValue)
        {
            Handle.GetDesc().SetClearValue(ClearValue);
        }
        m_ImportedResources.emplace(Id);
    }

    void RenderGraphStorage::DeclareResourceView(
        const ResourceViewId&   ViewId,
        const ResourceViewDesc& Desc)
    {
        auto Iter = m_Resources.find(ViewId.GetResource());
        NEON_ASSERT_MSG(Iter != m_Resources.end(), "Resource doesn't exists");

        auto& Views = Iter->second.GetViews();
        NEON_ASSERT_MSG(Views.emplace(ViewId.Get(), ResourceViewDescriptorDesc{ {}, Desc }).second, "Resource view already exists exists");
    }

    //

    void RenderGraphStorage::ImportRootSignature(
        const ResourceId&         Id,
        const RHI::RootSignature& RootSignature)
    {
        std::lock_guard Lock(m_PipelineMutex);
        m_RootSignatures.emplace(Id, RootSignature);
        RHI::RenameObject(RootSignature.Get(), Id.GetName());
    }

    void RenderGraphStorage::ImportPipelineState(
        const ResourceId&         Id,
        const RHI::PipelineState& PipelineState)
    {
        std::lock_guard Lock(m_PipelineMutex);
        m_PipelineStates.emplace(Id, PipelineState);
        RHI::RenameObject(PipelineState.Get(), Id.GetName());
    }

    //

    void RenderGraphStorage::RellocateResource(
        ResourceHandle& Handle)
    {
        auto& Desc = Handle.GetDesc();
        if (Handle.IsWindowSizedTexture())
        {
            auto& WindowSize = m_Display->GetSize();
            Desc.Width       = WindowSize.Width();
            Desc.Height      = WindowSize.Height();
        }

        auto Iter = std::ranges::find_if(
            m_InactiveResources,
            [&Desc](const ResourceHandle& OtherHandle)
            {
                return Desc == OtherHandle.GetDesc();
            });

        if (Iter != m_InactiveResources.end())
        {
            auto& NewDesc = Iter->GetDesc();

            Desc.ClearValue = NewDesc.ClearValue;

            Handle.Set(Iter->Get());
            m_InactiveResources.erase(Iter);
        }
        else
        {

            RHI::GraphicsResource::Ptr Res;
            if (Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
            {
                switch (Handle.GetBufferType())
                {
                case RHI::GraphicsBufferType::Default:
                    Res = RHI::GraphicsDefaultBuffer::Instantiate(Desc.Width, Desc.Alignment, Desc.Flags);
                    break;
                case RHI::GraphicsBufferType::Upload:
                    Res = RHI::GraphicsUploadBuffer::Instantiate(Desc.Width, Desc.Alignment, Desc.Flags);
                    break;
                case RHI::GraphicsBufferType::Readback:
                    Res = RHI::GraphicsReadbackBuffer::Instantiate(Desc.Width, Desc.Alignment, Desc.Flags);
                    break;
                default:
                    std::unreachable();
                }
            }
            else
            {
                D3D12_CLEAR_VALUE  ClearValue;
                D3D12_CLEAR_VALUE* ClearValuePtr = nullptr;
                if (Desc.ClearValue)
                {
                    ClearValuePtr     = &ClearValue;
                    ClearValue.Format = Desc.ClearValue->Format;
                    std::ranges::copy(Desc.ClearValue->Color, std::begin(ClearValue.Color));
                }

                Res = RHI::GraphicsTexture::Instantiate(
                    static_cast<D3D12_RESOURCE_DESC&>(Desc),
                    D3D12_RESOURCE_STATE_COMMON,
                    ClearValuePtr);
            }

            Handle.Set(Res);
        }

        auto D3dResource                        = Handle.Get()->Get();
        static_cast<D3D12_RESOURCE_DESC&>(Desc) = D3dResource->GetDesc();
        RHI::RenameObject(D3dResource, Handle.GetId().GetName());
    }

    void RenderGraphStorage::FreeResource(
        const ResourceHandle& Handle)
    {
        m_InactiveResources.emplace_back(Handle);
    }

    void RenderGraphStorage::FlushResources()
    {
        for (auto& Handle : m_InactiveResources)
        {
            m_Display->DiscardResource(Handle.Get());
        }
        m_InactiveResources.clear();
    }

    //

    void RenderGraphStorage::CreateViews(
        ResourceHandle& Handle)
    {
        auto& Resource = Handle.Get();
        auto& Views    = Handle.GetViews();

        for (auto& View : Views)
        {
            auto& [ViewDescHandle, ViewDesc] = View.second;
            D3D12_DESCRIPTOR_HEAP_TYPE Type;

            switch (ResourceViewDescType(ViewDesc.index()))
            {
            case ResourceViewDescType::ConstantBuffer:
            case ResourceViewDescType::ShaderResource:
            case ResourceViewDescType::UnorderedAccess:
                Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                break;
            case ResourceViewDescType::RenderTarget:
                Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                break;
            case ResourceViewDescType::DepthStencil:
                Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
                break;
            default:
                std::unreachable();
            }

            ViewDescHandle = m_Display->AllocateFromDescriptorHeapManager(
                Type, 1);

            switch (ResourceViewDescType(ViewDesc.index()))
            {
            case ResourceViewDescType::ConstantBuffer:
            {
                using DescType = ConstantBufferViewDesc;
                auto& HeapDesc = std::get<DescType>(ViewDesc);

                ViewDescHandle.Heap->CreateConstantBufferView(
                    ViewDescHandle.Offset,
                    std::static_pointer_cast<RHI::GraphicsBuffer>(Handle.Get())->GetGpuAddress() + HeapDesc.ViewOffset,
                    HeapDesc.ViewSize);
                break;
            }

            case ResourceViewDescType::ShaderResource:
            {
                using DescType = ShaderResourceViewDesc;
                auto& HeapDesc = std::get<DescType>(ViewDesc).HeapDesc;

                ViewDescHandle.Heap->CreateShaderResourceView(
                    ViewDescHandle.Offset,
                    Handle.Get()->Get(),
                    HeapDesc.has_value() ? &*HeapDesc : nullptr);
                break;
            }

            case ResourceViewDescType::UnorderedAccess:
            {
                using DescType = UnorderedAccessViewDesc;
                auto& HeapDesc = std::get<DescType>(ViewDesc).HeapDesc;

                ViewDescHandle.Heap->CreateUnorderedAccessView(
                    ViewDescHandle.Offset,
                    Handle.Get()->Get(),
                    HeapDesc.has_value() ? &*HeapDesc : nullptr);
                break;
            }

            case ResourceViewDescType::RenderTarget:
            {

                using DescType = RenderTargetViewDesc;
                auto& HeapDesc = std::get<DescType>(ViewDesc).HeapDesc;

                ViewDescHandle.Heap->CreateRenderTargetView(
                    ViewDescHandle.Offset,
                    Handle.Get()->Get(),
                    HeapDesc.has_value() ? &*HeapDesc : nullptr);
                break;
            }

            case ResourceViewDescType::DepthStencil:
            {
                using DescType = DepthStencilViewDesc;
                auto& HeapDesc = std::get<DescType>(ViewDesc).HeapDesc;

                ViewDescHandle.Heap->CreateDepthStencilView(
                    ViewDescHandle.Offset,
                    Handle.Get()->Get(),
                    HeapDesc.has_value() ? &*HeapDesc : nullptr);
                break;
            }
            break;
            }
        }
    }
} // namespace Renderer::RG
