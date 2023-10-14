#include <EnginePCH.hpp>
#include <RenderGraph/Common.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    ResourceHandle::ResourceHandle(
        const ResourceId&             Id,
        const Ptr<RHI::IGpuResource>& Texture,
        const RHI::ClearOperationOpt& ClearValue) :
        m_Id(Id),
        m_Resource(Texture),
        m_Desc(Texture ? Texture->GetDesc() : RHI::ResourceDesc{})
    {
        m_Flags.Set(EResourceFlags::Imported);
        m_Desc.ClearValue = ClearValue;
    }

    ResourceHandle::ResourceHandle(
        const ResourceId&             Id,
        const Ptr<RHI::IGpuResource>& Buffer,
        RHI::GraphicsBufferType       BufferType) :
        m_Id(Id),
        m_Resource(Buffer),
        m_Desc(Buffer->GetDesc())
    {
        m_Flags.Set(EResourceFlags::Imported);
    }

    ResourceHandle::ResourceHandle(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags) :
        m_Id(Id),
        m_Desc(Desc),
        m_Flags(Flags)
    {
    }

    const ResourceId& ResourceHandle::GetId() const noexcept
    {
        return m_Id;
    }

    void ResourceHandle::CreateView(
        const ResourceViewId&          ViewId,
        const RHI::DescriptorViewDesc& Desc,
        const RHI::SubresourceView&    Subresource)
    {
        NEON_ASSERT(!m_ViewMap.contains(ViewId), "View name already exists");

        m_ViewMap[ViewId] = {
            .Desc             = Desc,
            .SubresourceIndex = Subresource.GetSubresourceIndex(
                m_Desc.Depth, m_Desc.MipLevels)
        };
    }

    auto ResourceHandle::GetViews() noexcept -> ResourceViewMapType&
    {
        return m_ViewMap;
    }

    auto ResourceHandle::GetViews() const noexcept -> const ResourceViewMapType&
    {
        return m_ViewMap;
    }

    RHI::ResourceDesc& ResourceHandle::GetDesc() noexcept
    {
        return m_Desc;
    }

    const RHI::ResourceDesc& ResourceHandle::GetDesc() const noexcept
    {
        return m_Desc;
    }

    const MResourceFlags& ResourceHandle::GetFlags() const noexcept
    {
        return m_Flags;
    }

    const Ptr<RHI::IGpuResource>& ResourceHandle::Get() const noexcept
    {
        return m_Resource;
    }

    void ResourceHandle::Set(
        const Ptr<RHI::IGpuResource>& Resource,
        bool                          Rename) noexcept
    {
        bool ShouldRename = Rename && Resource && m_Resource != Resource;
        m_Resource        = Resource;
        if (ShouldRename)
        {
            SetName();
        }
    }

    RHI::GpuTexture ResourceHandle::AsTexture() const noexcept
    {
        return RHI::GpuTexture(m_Resource.get());
    }

    RHI::GpuBuffer ResourceHandle::AsBuffer() const noexcept
    {
        return RHI::GpuBuffer(m_Resource.get());
    }

    RHI::GpuUploadBuffer ResourceHandle::AsUploadBuffer() const noexcept
    {
        return RHI::GpuUploadBuffer(m_Resource.get());
    }

    RHI::GpuReadbackBuffer ResourceHandle::AsReadbackBuffer() const noexcept
    {
        return RHI::GpuReadbackBuffer(m_Resource.get());
    }

    bool ResourceHandle::IsWindowSizedTexture() const noexcept
    {
        return m_Flags.Test(EResourceFlags::WindowSizeDependent);
    }

    void ResourceHandle::MakeWindowSizedTexture(
        bool State) noexcept
    {
        m_Flags.Set(EResourceFlags::WindowSizeDependent, State);
    }

    bool ResourceHandle::IsImported() const noexcept
    {
        return m_Flags.Test(EResourceFlags::Imported);
    }

    void ResourceHandle::SetName()
    {
#ifndef NEON_DIST
        if (m_Resource)
        {
            RHI::RenameObject(m_Resource.get(), StringUtils::Transform<String>(m_Id.GetName()).c_str());
        }
#endif
    }
} // namespace Neon::RG