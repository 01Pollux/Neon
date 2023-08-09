#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Storage.hpp>

#include <RHI/GlobalDescriptors.hpp>
#include <RHI/Swapchain.hpp>

#include <RHI/Resource/Views/ConstantBuffer.hpp>
#include <RHI/Resource/Views/ShaderResource.hpp>
#include <RHI/Resource/Views/UnorderedAccess.hpp>
#include <RHI/Resource/Views/RenderTarget.hpp>
#include <RHI/Resource/Views/DepthStencil.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    void GraphStorage::Reset()
    {
        FlushResources();
        m_Resources.clear();
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

    //

    void GraphStorage::DeclareBuffer(
        const ResourceId&       Id,
        const RHI::BufferDesc&  Desc,
        RHI::GraphicsBufferType Type)
    {
        auto HandleIter = m_Resources.emplace(Id, ResourceHandle(Id, RHI::ResourceDesc::Buffer(Desc.Size, Desc.Alignment, Desc.Flags), {}, Type));
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
#ifndef NEON_DIST
        RHI::RenameObject(Handle.get(), Id.GetName().c_str());
#endif

        m_ImportedResources.emplace(Id);
    }

    void GraphStorage::ImportTexture(
        const ResourceId&             Id,
        const Ptr<RHI::ITexture>&     Texture,
        const RHI::ClearOperationOpt& ClearValue)
    {
        NEON_ASSERT(!ContainsResource(Id), "Resource already exists");
        auto& Handle = m_Resources.emplace(Id, ResourceHandle(Id, Texture, ClearValue)).first->second.Get();
#ifndef NEON_DIST
        RHI::RenameObject(Handle.get(), Id.GetName().c_str());
#endif
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

    void GraphStorage::RellocateResource(
        ResourceHandle& Handle)
    {
        auto& Desc = Handle.GetDesc();
        if (Handle.IsWindowSizedTexture())
        {
            auto& WindowSize = RHI::ISwapchain::Get()->GetSize();

            Desc.Width  = std::max(1, WindowSize.Width());
            Desc.Height = std::max(1, WindowSize.Height());
        }

        auto Iter = std::ranges::find_if(
            m_InactiveResources,
            [&Desc](const ResourceHandle& Handle)
            {
                return Handle.GetDesc() == Desc;
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
                    .Flags     = Desc.Flags,
                    .UsePool   = false
                };
                switch (Handle.GetBufferType())
                {
                case RHI::GraphicsBufferType::Default:
                    Res.reset(RHI::IBuffer::Create(BufferDesc));
                    break;
                case RHI::GraphicsBufferType::Upload:
                    Res.reset(RHI::IUploadBuffer::Create(BufferDesc));
                    break;
                case RHI::GraphicsBufferType::Readback:
                    Res.reset(RHI::IReadbackBuffer::Create(BufferDesc));
                    break;
                default:
                    std::unreachable();
                }
            }
            else
            {
                Res.reset(RHI::ITexture::Create(Desc));
            }

            Handle.Set(Res);
            Desc = Res->GetDesc();
        }

#ifndef NEON_DIST
        RHI::RenameObject(Handle.Get().get(), Handle.GetId().GetName().c_str());
#endif
    }

    void GraphStorage::FreeResource(
        const ResourceHandle& Handle)
    {
        m_InactiveResources.emplace_back(Handle);
    }

    void GraphStorage::FlushResources()
    {
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
                        RHI::Views::ConstantBuffer View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);
                        View.Bind(Desc);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const std::optional<RHI::SRVDesc>& Desc)
                    {
                        RHI::Views::ShaderResource View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const std::optional<RHI::UAVDesc>& Desc)
                    {
                        RHI::Views::UnorderedAccess View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const std::optional<RHI::RTVDesc>& Desc)
                    {
                        RHI::Views::RenderTarget View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::RenderTargetView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const std::optional<RHI::DSVDesc>& Desc)
                    {
                        RHI::Views::DepthStencil View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::DepthStencilView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    } },
                ViewDesc);
        }
    }
} // namespace Neon::RG