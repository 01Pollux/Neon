#include <EnginePCH.hpp>
#include <Math/Common.hpp>

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
    GraphStorage::GraphStorage() :
        m_CameraFrameData(RHI::IGpuResource::Create(
            RHI::ResourceDesc::BufferUpload(
                Math::AlignUp(sizeof(CameraFrameData), 255))))
    {
    }

    void GraphStorage::Reset()
    {
        m_Resources.clear();

        // Importing null output image to allow to use it in the graph
        ImportTexture(
            ResourceResolver::GetOutputImageTag(),
            nullptr);
        auto& Resource = GetResourceMut(ResourceResolver::GetOutputImageTag());
        Resource.MakeWindowSizedTexture(true);
        Resource.GetDesc() = RHI::ResourceDesc::Tex2D(
            ResourceResolver::GetSwapchainFormat(),
            1, 1, 1);
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

    Size2I GraphStorage::GetResourceSize(
        const ResourceId& Id) const
    {
        // We are using resource's desc since it is possible to change resource's dimension during graph execution
        // But the chance will only apply when the graph rerun again
        auto&  Resoure = GetResource(Id).Get();
        Size2I Size{ 1, 1 };
        if (Resoure)
        {
            auto& Desc = Resoure->GetDesc();
            Size       = { Desc.Width, Desc.Height };
        }
        return Size;
    }

    const RHI::DescriptorViewDesc& GraphStorage::GetResourceView(
        const ResourceViewId&     ViewId,
        RHI::CpuDescriptorHandle* CpuHandle) const
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
                return ViewInfo.second;
            }
        }

        NEON_ASSERT(false, "Resource view doesn't exists");
        std::unreachable();
    }

    const ResourceHandle& GraphStorage::GetOutputImage() const
    {
        return GetResource(ResourceResolver::GetOutputImageTag());
    }

    Size2I GraphStorage::GetOutputImageSize() const
    {
        return GetResourceSize(ResourceResolver::GetOutputImageTag());
    }

    void GraphStorage::SetOutputImageSize(
        std::optional<Size2I> Size)
    {
        auto& Resource = GetResourceMut(ResourceResolver::GetOutputImageTag());
        Resource.MakeWindowSizedTexture(!Size.has_value());
        if (Size)
        {
            auto& Desc  = Resource.GetDesc();
            Desc.Width  = Size->Width();
            Desc.Height = Size->Height();
        }
    }

    //

    CameraFrameData& GraphStorage::MapFrameData() const
    {
        return *m_CameraFrameData->Map<CameraFrameData>();
    }

    void GraphStorage::UnmapFrameData() const
    {
        m_CameraFrameData->Unmap();
    }

    RHI::GpuResourceHandle GraphStorage::GetFrameDataHandle() const
    {
        return m_CameraFrameData->GetHandle();
    }

    //

    void GraphStorage::DeclareResource(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags)
    {
        auto HandleIter = m_Resources.emplace(Id, ResourceHandle(Id, Desc, std::move(Flags)));
        NEON_ASSERT(HandleIter.second, "Resource already exists");
    }

    void GraphStorage::ImportBuffer(
        const ResourceId&             Id,
        const Ptr<RHI::IGpuResource>& Buffer,
        RHI::GraphicsBufferType       BufferType)
    {
        NEON_ASSERT(!ContainsResource(Id), "Resource already exists");
        auto& Handle = m_Resources.emplace(Id, ResourceHandle(Id, Buffer, BufferType)).first->second.Get();
#ifndef NEON_DIST
        RHI::RenameObject(Handle.get(), Id.GetName());
#endif

        m_ImportedResources.emplace(Id);
    }

    void GraphStorage::ImportTexture(
        const ResourceId&             Id,
        const Ptr<RHI::IGpuResource>& Texture,
        const RHI::ClearOperationOpt& ClearValue)
    {
        NEON_ASSERT(!ContainsResource(Id), "Resource already exists");
        auto& Handle = m_Resources.emplace(Id, ResourceHandle(Id, Texture, ClearValue)).first->second.Get();
#ifndef NEON_DIST
        if (Handle.get()) [[likely]]
        {
            RHI::RenameObject(Handle.get(), Id.GetName());
        }
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

    void GraphStorage::NewOutputImage()
    {
        auto& OutputImage = GetResourceMut(ResourceResolver::GetOutputImageTag());

        auto& Desc = OutputImage.GetDesc();
        if (OutputImage.IsWindowSizedTexture())
        {
            auto& WindowSize = RHI::ISwapchain::Get()->GetSize();
            Desc.Width       = WindowSize.Width();
            Desc.Height      = WindowSize.Height();
        }

        Desc.SetClearValue(Colors::Magenta);
        Desc.Flags.Set(RHI::EResourceFlags::AllowRenderTarget);

        OutputImage.Set(Ptr<RHI::IGpuResource>(RHI::IGpuResource::Create(
            Desc,
            { .Name = ResourceResolver::GetOutputImageTag().GetName().c_str() })));
    }

    void GraphStorage::ReallocateResource(
        ResourceHandle& Handle)
    {
        auto& Desc       = Handle.GetDesc();
        bool  WasChanged = true;

        if (Handle.IsWindowSizedTexture())
        {
            auto Size = GetOutputImageSize();

            WasChanged = Desc.Width != Size.Width() ||
                         Desc.Height != Size.Height();

            Desc.Width  = Size.Width();
            Desc.Height = Size.Height();
        }

        if (!Handle.Get() || WasChanged)
        {
            Ptr<RHI::IGpuResource> Res;

            // We have to reset mip levels to 0 since we don't know how many mip levels the resource will have
            Desc.MipLevels = 0;
            Res.reset(RHI::IGpuResource::Create(Desc));

            Handle.Set(Res);
            Desc = Res->GetDesc();

#ifndef NEON_DIST
            RHI::RenameObject(Handle.Get().get(), Handle.GetId().GetName());
#endif
        }
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
                    [&ViewDescHandle, Resource, this](const RHI::SRVDescOpt& Desc)
                    {
                        RHI::Views::ShaderResource View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const RHI::UAVDescOpt& Desc)
                    {
                        RHI::Views::UnorderedAccess View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const RHI::RTVDescOpt& Desc)
                    {
                        RHI::Views::RenderTarget View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::RenderTargetView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    },
                    [&ViewDescHandle, Resource, this](const RHI::DSVDescOpt& Desc)
                    {
                        RHI::Views::DepthStencil View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::DepthStencilView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDescHandle = View;
                    } },
                ViewDesc);
        }
    }
} // namespace Neon::RG