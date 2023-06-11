#include <EnginePCH.hpp>
#include <Renderer/RG/Common.hpp>

namespace Neon::RG
{
    ResourceHandle::ResourceHandle(
        const ResourceId&             Id,
        const Ptr<RHI::ITexture>&     Texture,
        const RHI::ClearOperationOpt& ClearValue) :
        m_Id(Id),
        m_Resource(Texture),
        m_Desc(Texture->GetDesc())
    {
        m_Flags.Set(EResourceFlags::Imported);
        m_Desc.ClearValue = ClearValue;
    }

    ResourceHandle::ResourceHandle(
        const ResourceId&        Id,
        const Ptr<RHI::IBuffer>& Buffer,
        RHI::GraphicsBufferType  BufferType) :
        m_Id(Id),
        m_Resource(Buffer),
        m_Desc(Buffer->GetDesc()),
        m_BufferType(BufferType)
    {
        m_Flags.Set(EResourceFlags::Imported);
    }

    ResourceHandle::ResourceHandle(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags,
        RHI::GraphicsBufferType  BufferType) :
        m_Id(Id),
        m_Desc(Desc),
        m_Flags(Flags),
        m_BufferType(BufferType)
    {
    }

    const ResourceId& ResourceHandle::GetId() const noexcept
    {
        return m_Id;
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

    const Ptr<RHI::IGpuResource>& ResourceHandle::Get() const noexcept
    {
        return m_Resource;
    }

    void ResourceHandle::Set(
        const Ptr<RHI::IGpuResource>& Resource) noexcept
    {
        m_Resource = Resource;
    }

    Ptr<RHI::ITexture> ResourceHandle::AsTexture() const noexcept
    {
        return std::dynamic_pointer_cast<RHI::ITexture>(Get());
    }

    Ptr<RHI::IBuffer> ResourceHandle::AsBuffer() const noexcept
    {
        return std::dynamic_pointer_cast<RHI::IBuffer>(Get());
    }

    RHI::GraphicsBufferType ResourceHandle::GetBufferType() const noexcept
    {
        return m_BufferType;
    }

    bool ResourceHandle::IsWindowSizedTexture() const noexcept
    {
        return m_Flags.Test(EResourceFlags::WindowSizeDependent);
    }

    bool ResourceHandle::IsImported() const noexcept
    {
        return m_Flags.Test(EResourceFlags::Imported);
    }
} // namespace Neon::RG