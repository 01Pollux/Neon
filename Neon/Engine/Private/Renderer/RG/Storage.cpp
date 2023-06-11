#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Storage.hpp>

#include <RHI/Resource/Views/ConstantBuffer.hpp>
#include <RHI/Resource/Views/ShaderResource.hpp>
#include <RHI/Resource/Views/UnorderedAccess.hpp>
#include <RHI/Resource/Views/RenderTarget.hpp>
#include <RHI/Resource/Views/DepthStencil.hpp>

#include <RHI/Swapchain.hpp>
#include <Window/Window.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    GraphStorage::GraphStorage(
        RHI::ISwapchain* Swapchain) :
        m_Swapchain(Swapchain)
    {
    }

    void GraphStorage::Reset()
    {
        FlushResources();
        m_Resources.clear();
        m_RootSignatures.clear();
        m_Shaders.clear();
        m_PipelineStates.clear();
    }

    bool GraphStorage::ContainsResource(
        const ResourceId& Id) const
    {
        return m_Resources.contains(Id);
    }

    bool GraphStorage::ContainsResourceView(
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

    ResourceHandle& GraphStorage::GetResourceMut(
        const ResourceId& Id)
    {
        auto Iter = m_Resources.find(Id);
        NEON_ASSERT(Iter != m_Resources.end(), "Resource doesn't exists");
        return Iter->second;
    }

    const ResourceHandle& GraphStorage::GetResource(
        const ResourceId& Id) const
    {
        auto Iter = m_Resources.find(Id);
        NEON_ASSERT(Iter != m_Resources.end(), "Resource doesn't exists");
        return Iter->second;
    }

    const RHI::DescriptorViewDesc& GraphStorage::GetResourceView(
        const ResourceViewId&     ViewId,
        RHI::CpuDescriptorHandle* CpuHandle,
        RHI::GpuDescriptorHandle* GpuHandle) const
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
                    *CpuHandle = ViewInfo.first.GetCpuHandle();
                }
                if (GpuHandle)
                {
                    *GpuHandle = ViewInfo.first.GetGpuHandle();
                }
                return ViewInfo.second;
            }
        }

        NEON_ASSERT(false, "Resource view doesn't exists");
        std::unreachable();
    }

    RHI::ISwapchain* GraphStorage::GetSwapchain() const
    {
        return m_Swapchain;
    }

    //

    const Ptr<RHI::IRootSignature>& GraphStorage::GetRootSignature(
        const ResourceId& Id) const
    {
        std::shared_lock Lock(m_RootSignaturesMutex);

        auto Iter = m_RootSignatures.find(Id);
        NEON_ASSERT(Iter != m_RootSignatures.end(), "Root signature doesn't exists");
        return Iter->second;
    }

    const Ptr<RHI::IShader>& GraphStorage::GetShader(
        const ResourceId& Id) const
    {
        std::shared_lock Lock(m_ShadersMutex);

        auto Iter = m_Shaders.find(Id);
        NEON_ASSERT(Iter != m_Shaders.end(), "Shader doesn't exists");
        return Iter->second;
    }

    const Ptr<RHI::IPipelineState>& GraphStorage::GetPipelineState(
        const ResourceId& Id) const
    {
        std::shared_lock Lock(m_PipelineStatesMutex);

        auto Iter = m_PipelineStates.find(Id);
        NEON_ASSERT(Iter != m_PipelineStates.end(), "Pipeline state doesn't exists");
        return Iter->second;
    }

    //

    void GraphStorage::DeclareBuffer(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        RHI::GraphicsBufferType  Type)
    {
        NEON_ASSERT(Desc.Type == RHI::ResourceType::Buffer);
        auto HandleIter = m_Resources.emplace(Id, ResourceHandle(Id, Desc, {}, Type));
        NEON_ASSERT(HandleIter.second, "Resource already exists");
    }

    void GraphStorage::DeclareTexture(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags)
    {
        NEON_ASSERT(Desc.Type != RHI::ResourceType::Buffer);
        auto HandleIter = m_Resources.emplace(Id, ResourceHandle(Id, Desc, std::move(Flags)));
        NEON_ASSERT(HandleIter.second, "Resource already exists");
    }

    void GraphStorage::ImportBuffer(
        const ResourceId&        Id,
        const Ptr<RHI::IBuffer>& Buffer,
        RHI::GraphicsBufferType  BufferType)
    {
        NEON_ASSERT(!ContainsResource(Id), "Resource already exists");
        auto& Handle = m_Resources.emplace(Id, ResourceHandle(Id, Buffer, BufferType)).first->second.Get();
        RHI::RenameObject(Handle.get(), Id.GetName());

        m_ImportedResources.emplace(Id);
    }

    void GraphStorage::ImportTexture(
        const ResourceId&             Id,
        const Ptr<RHI::ITexture>&     Texture,
        const RHI::ClearOperationOpt& ClearValue)
    {
        NEON_ASSERT(!ContainsResource(Id), "Resource already exists");
        auto& Handle = m_Resources.emplace(Id, ResourceHandle(Id, Texture, ClearValue)).first->second;
        RHI::RenameObject(Handle.Get().get(), Id.GetName());

        // TODO: do we need to set clear value here?
        if (ClearValue)
        {
            Handle.GetDesc().ClearValue = ClearValue;
        }
        m_ImportedResources.emplace(Id);
    }

    void GraphStorage::DeclareResourceView(
        const ResourceViewId&          ViewId,
        const RHI::DescriptorViewDesc& Desc)
    {
        auto Iter = m_Resources.find(ViewId.GetResource());
        NEON_ASSERT(Iter != m_Resources.end(), "Resource doesn't exists");

        auto& Views = Iter->second.GetViews();
        NEON_ASSERT(Views.emplace(ViewId.Get(), ResourceHandle::ViewDesc{ {}, Desc }).second, "Resource view already exists exists");
    }

    //

    void GraphStorage::ImportRootSignature(
        const ResourceId&               Id,
        const Ptr<RHI::IRootSignature>& RootSignature)
    {
        std::unique_lock Lock(m_RootSignaturesMutex);
        m_RootSignatures.emplace(Id, RootSignature);
        RHI::RenameObject(RootSignature.get(), Id.GetName());
    }

    void GraphStorage::ImportShader(
        const ResourceId&        Id,
        const Ptr<RHI::IShader>& Shader)
    {
        std::unique_lock Lock(m_ShadersMutex);
        m_Shaders.emplace(Id, Shader);
    }

    void GraphStorage::ImportPipelineState(
        const ResourceId&               Id,
        const Ptr<RHI::IPipelineState>& PipelineState)
    {
        std::unique_lock Lock(m_PipelineStatesMutex);
        m_PipelineStates.emplace(Id, PipelineState);
        RHI::RenameObject(PipelineState.get(), Id.GetName());
    }

    //

    void GraphStorage::RellocateResource(
        ResourceHandle& Handle)
    {
        auto& Desc = Handle.GetDesc();
        if (Handle.IsWindowSizedTexture())
        {
            auto WindowSize = m_Swapchain->GetWindow()->GetSize();
            Desc.Width      = WindowSize.Width();
            Desc.Height     = WindowSize.Height();
        }

        auto Iter = std::ranges::find_if(
            m_InactiveResources,
            [&Desc](const ResourceHandle& OtherHandle)
            {
                return false;
                // return Desc == OtherHandle.GetDesc();
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

            Ptr<RHI::IGpuResource> Res;
            if (Desc.Type == RHI::ResourceType::Buffer)
            {
                RHI::BufferDesc BufferDesc{
                    .Size      = Desc.Width,
                    .Alignment = Desc.Alignment,
                    .Flags     = Desc.Flags
                };
                switch (Handle.GetBufferType())
                {
                case RHI::GraphicsBufferType::Default:
                    Res.reset(RHI::IBuffer::Create(m_Swapchain, BufferDesc));
                    break;
                case RHI::GraphicsBufferType::Upload:
                    Res.reset(RHI::IUploadBuffer::Create(m_Swapchain, BufferDesc));
                    break;
                case RHI::GraphicsBufferType::Readback:
                    Res.reset(RHI::IReadbackBuffer::Create(m_Swapchain, BufferDesc));
                    break;
                default:
                    std::unreachable();
                }
            }
            else
            {
                Res.reset(RHI::ITexture::Create(m_Swapchain, Desc));
            }

            Handle.Set(Res);
        }

        // TODO: remove this
        // auto D3dResource                        = Handle.Get()->Get();
        // static_cast<D3D12_RESOURCE_DESC&>(Desc) = D3dResource->GetDesc();
        // RHI::RenameObject(D3dResource, Handle.GetId().GetName());
    }

    void GraphStorage::FreeResource(
        const ResourceHandle& Handle)
    {
        m_InactiveResources.emplace_back(Handle);
    }

    void GraphStorage::FlushResources()
    {
        // TODO: handle resource discard
        m_InactiveResources.clear();
    }

    //

    void GraphStorage::CreateViews(
        ResourceHandle& Handle)
    {
        auto& Resource = Handle.Get();
        auto& Views    = Handle.GetViews();

        for (auto& View : Views)
        {
            auto& [ViewDescHandle, ViewDesc] = View.second;

            // TODO: batch allocations
            std::visit(
                VariantVisitor{
                    [](const std::monostate&)
                    {
                        NEON_ASSERT(false, "Invalid view type");
                    },
                    [&ViewDescHandle, this](const RHI::CBVDesc& Desc)
                    {
                        auto View = RHI::Views::ConstantBuffer(
                            m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::ResourceView, true),
                            1);
                        View.Bind(Desc);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const std::optional<RHI::SRVDesc>& Desc)
                    {
                        auto View = RHI::Views::ShaderResource(
                            m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::ResourceView, true),
                            1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const std::optional<RHI::UAVDesc>& Desc)
                    {
                        auto View = RHI::Views::UnorderedAccess(
                            m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::ResourceView, true),
                            1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const std::optional<RHI::RTVDesc>& Desc)
                    {
                        auto View = RHI::Views::RenderTarget(
                            m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::RenderTargetView, true),
                            1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const std::optional<RHI::DSVDesc>& Desc)
                    {
                        auto View = RHI::Views::DepthStencil(
                            m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::DepthStencilView, true),
                            1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    } },
                ViewDesc);
        }
    }
} // namespace Neon::RG