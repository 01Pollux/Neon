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
                Math::AlignUp(sizeof(CameraFrameData) * RHI::ISwapchain::Get()->GetFrameCount(), 256)))),
        m_CameraFrameDataPtr(m_CameraFrameData->Map()),
        m_SceneContext(*this)
    {
    }

    void GraphStorage::Reset()
    {
        m_Resources.clear();

        auto OutputImageId = ResourceResolver::GetOutputImage();
        // Importing null output image to allow to use it in the graph
        ImportTexture(
            OutputImageId,
            nullptr);

        auto& Resource = GetResourceMut(OutputImageId);
        Resource.MakeWindowSizedTexture(true);
        Resource.GetDesc() = RHI::ResourceDesc::Tex2D(
            ResourceResolver::GetSwapchainFormat(),
            1, 1, 1);
    }

    //

    bool GraphStorage::ContainsResource(
        const ResourceId& Id) const
    {
        return m_Resources.contains(Id);
    }

    bool GraphStorage::ContainsResourceView(
        const ResourceViewId& ViewId)
    {
        auto Iter = m_Resources.find(ViewId.GetResource());
        if (Iter != m_Resources.end())
        {
            return Iter->second.GetViews().contains(ViewId);
        }
        return false;
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
        ResourceViewId            ViewId,
        RHI::CpuDescriptorHandle* CpuHandle,
        uint32_t*                 SubresourceIndex) const
    {
        auto Iter = m_Resources.find(ViewId.GetResource());
        if (Iter != m_Resources.end())
        {
            auto& Views    = Iter->second.GetViews();
            auto  ViewIter = Views.find(ViewId);

            if (ViewIter != Views.end())
            {
                auto& ViewInfo = ViewIter->second;
                if (CpuHandle)
                {
                    *CpuHandle = ViewInfo.Handle.GetCpuHandle();
                }
                if (SubresourceIndex)
                {
                    *SubresourceIndex = ViewInfo.SubresourceIndex;
                }
                return ViewInfo.Desc;
            }
        }

        NEON_ASSERT(false, "Resource view doesn't exists");
        std::unreachable();
    }

    RHI::CpuDescriptorHandle GraphStorage::GetResourceViewHandle(
        ResourceViewId ViewId) const
    {
        RHI::CpuDescriptorHandle Handle;
        GetResourceView(ViewId, &Handle);
        return Handle;
    }

    const ResourceHandle& GraphStorage::GetOutputImage() const
    {
        return GetResource(ResourceResolver::GetOutputImage());
    }

    Size2I GraphStorage::GetOutputImageSize() const
    {
        return GetResourceSize(ResourceResolver::GetOutputImage());
    }

    void GraphStorage::SetOutputImageSize(
        std::optional<Size2I> Size)
    {
        auto& Resource = GetResourceMut(ResourceResolver::GetOutputImage());
        Resource.MakeWindowSizedTexture(!Size.has_value());
        if (Size)
        {
            auto& Desc  = Resource.GetDesc();
            Desc.Width  = Size->Width();
            Desc.Height = Size->Height();
        }
    }

    //

    const CameraFrameData& GraphStorage::GetFrameData() const
    {
        return *std::bit_cast<const CameraFrameData*>(m_CameraFrameDataPtr + sizeof(CameraFrameData) * RHI::ISwapchain::Get()->GetFrameIndex());
    }

    CameraFrameData& GraphStorage::GetFrameData()
    {
        return *std::bit_cast<CameraFrameData*>(m_CameraFrameDataPtr + sizeof(CameraFrameData) * RHI::ISwapchain::Get()->GetFrameIndex());
    }

    RHI::GpuResourceHandle GraphStorage::GetFrameDataHandle() const
    {
        return m_CameraFrameData->GetHandle();
    }

    const SceneContext& GraphStorage::GetSceneContext() const noexcept
    {
        return m_SceneContext;
    }

    SceneContext& GraphStorage::GetSceneContext() noexcept
    {
        return m_SceneContext;
    }

    //

    void GraphStorage::DeclareResource(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags)
    {
        NEON_ASSERT(!ContainsResource(Id), "Resource already exists");
        m_Resources.emplace(Id, ResourceHandle(Id, Desc, std::move(Flags)));
    }

    void GraphStorage::ImportBuffer(
        const ResourceId&             Id,
        const Ptr<RHI::IGpuResource>& Buffer,
        RHI::GraphicsBufferType       BufferType)
    {
        NEON_ASSERT(!ContainsResource(Id), "Resource already exists");

        auto& Handle = m_Resources.emplace(Id, ResourceHandle(Id, Buffer, BufferType)).first->second;
        m_ImportedResources.emplace(Id);

        Handle.SetName();
    }

    void GraphStorage::ImportTexture(
        const ResourceId&             Id,
        const Ptr<RHI::IGpuResource>& Texture,
        const RHI::ClearOperationOpt& ClearValue)
    {
        NEON_ASSERT(!ContainsResource(Id), "Resource already exists");

        auto& Handle = m_Resources.emplace(Id, ResourceHandle(Id, Texture, ClearValue)).first->second;
        m_ImportedResources.emplace(Id);

        Handle.SetName();
    }

    void GraphStorage::DeclareResourceView(
        const ResourceViewId&          ViewId,
        const RHI::DescriptorViewDesc& Desc,
        SubresourceView                Subresource)
    {
        auto Iter = m_Resources.find(ViewId.GetResource());
        NEON_ASSERT(Iter != m_Resources.end(), "Resource doesn't exists");
        Iter->second.CreateView(ViewId, Desc, Subresource);
    }

    //

    void GraphStorage::UpdateOutputImage(
        const Size2I& Size)
    {
        auto& OutputImage = GetResourceMut(ResourceResolver::GetOutputImage());
        auto& Desc        = OutputImage.GetDesc();

        bool WasChanged = false;
        if (OutputImage.IsWindowSizedTexture())
        {
            WasChanged = Desc.Width != Size.Width() ||
                         Desc.Height != Size.Height();

            Desc.Width  = Size.Width();
            Desc.Height = Size.Height();
        }

        if (!OutputImage.Get() || WasChanged)
        {
            Ptr<RHI::IGpuResource> Res;

            // We have to reset mip levels to 0 since we don't know how many mip levels the resource will have
            Desc.MipLevels = 0;
            Res.reset(RHI::IGpuResource::Create(Desc));

            OutputImage.Set(Res);
            Desc = Res->GetDesc();
        }
    }

    void GraphStorage::ReallocateResource(
        ResourceHandle& Handle)
    {
        auto& Desc       = Handle.GetDesc();
        bool  WasChanged = false;

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
            auto& ViewDesc = View.second;

            // TODO: batch allocations
            std::visit(
                VariantVisitor{
                    [](const std::monostate&)
                    {
                        NEON_ASSERT(false, "Invalid view type");
                    },
                    [&ViewDesc, this](const RHI::CBVDesc& Desc)
                    {
                        RHI::Views::ConstantBuffer View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);
                        View.Bind(Desc);
                        ViewDesc.Handle = View;
                    },
                    [&ViewDesc, Resource, this](const RHI::SRVDescOpt& Desc)
                    {
                        RHI::Views::ShaderResource View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDesc.Handle = View;
                    },
                    [&ViewDesc, Resource, this](const RHI::UAVDescOpt& Desc)
                    {
                        RHI::Views::UnorderedAccess View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDesc.Handle = View;
                    },
                    [&ViewDesc, Resource, this](const RHI::RTVDescOpt& Desc)
                    {
                        RHI::Views::RenderTarget View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::RenderTargetView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDesc.Handle = View;
                    },
                    [&ViewDesc, Resource, this](const RHI::DSVDescOpt& Desc)
                    {
                        RHI::Views::DepthStencil View = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::DepthStencilView)->Allocate(1);
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                        ViewDesc.Handle = View;
                    } },
                ViewDesc.Desc);
        }
    }

    void GraphStorage::PrepareDispatch()
    {
        CreateViews(GetResourceMut(ResourceResolver::GetOutputImage()));
    }
} // namespace Neon::RG