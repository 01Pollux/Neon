#include <EnginePCH.hpp>
#include <Renderer/RenderGraph/ResourceId.hpp>

namespace Renderer::RG
{
    ResourceDesc::ResourceDesc(
        CD3DX12_RESOURCE_DESC&& Other)
    {
        *static_cast<CD3DX12_RESOURCE_DESC*>(this) = std::move(Other);
    }

    void ResourceDesc::SetClearValue(
        DXGI_FORMAT   Format,
        const Color4& Color)
    {
        ClearValue = ClearOperation{
            .Format = Format,
            .Color  = Color
        };
    }

    void ResourceDesc::SetClearValue(
        DXGI_FORMAT Format,
        float       Depth,
        uint8_t     Stencil)
    {
        ClearValue = ClearOperation{
            .Format       = Format,
            .DepthStencil = { Depth, Stencil }
        };
    }

    void ResourceDesc::SetClearValue(
        const Color4& Color)
    {
        SetClearValue(ClearOperation{ .Format = Format, .Color = { Color } });
    }

    void ResourceDesc::SetClearValue(
        float   Depth,
        uint8_t Stencil)
    {
        SetClearValue(ClearOperation{ .Format = Format, .DepthStencil = { Depth, Stencil } });
    }

    void ResourceDesc::SetClearValue(
        const ClearOperationOpt& Op)
    {
        ClearValue = Op;
    }

    void ResourceDesc::UnsetClearValue()
    {
        ClearValue.reset();
    }

    //

    ResourceHandle::ResourceHandle(
        const ResourceId&                 Id,
        const RHI::GraphicsResource::Ptr& Resource,
        const ClearOperationOpt&          ClearValue) :
        m_Id(Id),
        m_Resource(Resource),
        m_Desc(Resource->Get()->GetDesc()),
        m_Flags(ResourceFlags::Internal_Imported)
    {
        m_Desc.ClearValue = ClearValue;
    }

    ResourceHandle::ResourceHandle(
        const ResourceId&                 Id,
        const RHI::GraphicsResource::Ptr& Resource,
        RHI::GraphicsBufferType           BufferType) :
        m_Id(Id),
        m_Resource(Resource),
        m_Desc(Resource->Get()->GetDesc()),
        m_Flags(ResourceFlags::Internal_Imported),
        m_BufferType(BufferType)
    {
    }

    ResourceHandle::ResourceHandle(
        const ResourceId&       Id,
        const ResourceDesc&     Desc,
        ResourceFlags           Flags,
        RHI::GraphicsBufferType BufferType) :
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

    ResourceDesc& ResourceHandle::GetDesc() noexcept
    {
        return m_Desc;
    }

    const ResourceDesc& ResourceHandle::GetDesc() const noexcept
    {
        return m_Desc;
    }

    const RHI::GraphicsResource::Ptr& ResourceHandle::Get() const noexcept
    {
        return m_Resource;
    }

    void ResourceHandle::Set(
        const RHI::GraphicsResource::Ptr& Resource) noexcept
    {
        m_Resource = Resource;
    }

    RHI::GraphicsBufferType ResourceHandle::GetBufferType() const noexcept
    {
        return m_BufferType;
    }

    bool ResourceHandle::IsWindowSizedTexture() const noexcept
    {
        return (m_Flags & ResourceFlags::WindowSizedTexture) != ResourceFlags::None;
    }

    bool ResourceHandle::IsImported() const noexcept
    {
        return (m_Flags & ResourceFlags::Internal_Imported) != ResourceFlags::None;
    }

    bool ResourceHandle::IsShaderVIsible() const noexcept
    {
        return (m_Flags & ResourceFlags::Internal_UsedInShader) != ResourceFlags::None;
    }
} // namespace Renderer::RG