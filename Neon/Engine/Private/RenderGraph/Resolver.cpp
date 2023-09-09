#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Resolver.hpp>
#include <RenderGraph/Storage.hpp>

#include <RHI/Swapchain.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    ResourceResolver::ResourceResolver(
        GraphStorage& Storage) :
        m_Storage(Storage)
    {
    }

    void ResourceResolver::CreateBuffer(
        const ResourceId&       Id,
        const RHI::BufferDesc&  Desc,
        RHI::GraphicsBufferType BufferType)
    {
        m_Storage.DeclareBuffer(Id, Desc, BufferType);
        m_ResourcesCreated.emplace(Id);
    }

    void ResourceResolver::CreateTexture(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags)
    {
        m_Storage.DeclareTexture(Id, Desc, std::move(Flags));
        m_ResourcesCreated.emplace(Id);
    }

    void ResourceResolver::CreateWindowTexture(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags)
    {
        Flags.Set(RG::EResourceFlags::WindowSizeDependent);
        CreateTexture(Id, Desc, std::move(Flags));
    }

    void ResourceResolver::WriteResourceEmpty(
        const ResourceId& Id)
    {
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesWritten.emplace(Id);
    }

    const ResourceViewId& ResourceResolver::WriteResource(
        const ResourceViewId&          ViewId,
        const RHI::DescriptorViewDesc& Desc)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesWritten.emplace(Id);

        RHI::MResourceState State;
        RHI::MResourceFlags Flags;

        std::visit(
            VariantVisitor{
                [&State, &Flags](const RHI::CBVDesc&)
                {
                    State.Set(RHI::EResourceState::ConstantBuffer);
                },
                [&State, &Flags](const RHI::UAVDescOpt& Desc)
                {
                    NEON_ASSERT(!Desc || !std::holds_alternative<std::monostate>(Desc->View), "View must be set");
                    State.Set(RHI::EResourceState::UnorderedAccess);
                    Flags.Set(RHI::EResourceFlags::AllowUnorderedAccess);
                },
                [&State, &Flags, this, &ViewId](const RHI::RTVDescOpt& Desc)
                {
                    NEON_ASSERT(!Desc || !std::holds_alternative<std::monostate>(Desc->View), "View must be set");
                    State.Set(RHI::EResourceState::RenderTarget);
                    Flags.Set(RHI::EResourceFlags::AllowRenderTarget);

                    m_RenderTargets.emplace_back(ViewId);
                },
                [&State, &Flags, this, &ViewId](const RHI::DSVDescOpt& Desc)
                {
                    NEON_ASSERT(!Desc || !std::holds_alternative<std::monostate>(Desc->View), "View must be set");
                    State.Set(RHI::EResourceState::DepthWrite);
                    Flags.Set(RHI::EResourceFlags::AllowDepthStencil);

                    m_DepthStencil = ViewId;
                },
                [&ViewId](const auto&)
                {
                    NEON_ASSERT(false, "Invalid descriptor view type to write to ({})", StringUtils::Transform<StringU8>(ViewId.GetName()));
                } },
            Desc);

        SetResourceState(ViewId, State, Flags);
        m_Storage.DeclareResourceView(ViewId, Desc);

        return ViewId;
    }

    const ResourceViewId& ResourceResolver::WriteDstResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesWritten.emplace(Id);

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(RHI::EResourceState::CopyDest), {});

        return ViewId;
    }

    void ResourceResolver::ReadResourceEmpty(
        const ResourceId& Id)
    {
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesRead.emplace(Id);
    }

    const ResourceViewId& ResourceResolver::ReadResource(
        const ResourceViewId&          ViewId,
        ResourceReadAccess             ReadAccess,
        const RHI::DescriptorViewDesc& Desc)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesRead.emplace(Id);

        RHI::MResourceState State{};
        RHI::MResourceFlags Flags{};

        std::visit(
            VariantVisitor{
                [&State, &Flags](const RHI::CBVDesc&)
                {
                    State.Set(RHI::EResourceState::ConstantBuffer);
                },
                [&State, &Flags](const RHI::SRVDescOpt& Desc)
                {
                    NEON_ASSERT(!Desc || !std::holds_alternative<std::monostate>(Desc->View), "View must be set");
                },
                [&State, &Flags](const RHI::UAVDescOpt& Desc)
                {
                    NEON_ASSERT(!Desc || !std::holds_alternative<std::monostate>(Desc->View), "View must be set");
                    State.Set(RHI::EResourceState::UnorderedAccess);
                    Flags.Set(RHI::EResourceFlags::AllowUnorderedAccess);
                },
                [&State, &Flags, this, &ViewId](const RHI::DSVDescOpt& Desc)
                {
                    NEON_ASSERT(!Desc || !std::holds_alternative<std::monostate>(Desc->View), "View must be set");
                    State.Set(RHI::EResourceState::DepthRead);
                    Flags.Set(RHI::EResourceFlags::AllowDepthStencil);

                    m_DepthStencil = ViewId;
                },
                [&ViewId](const auto&)
                {
                    NEON_ASSERT(false, "Invalid descriptor view type to read from ({})", StringUtils::Transform<StringU8>(ViewId.GetName()));
                } },
            Desc);

        switch (ReadAccess)
        {
        case ResourceReadAccess::PixelShader:
            State.Set(RHI::EResourceState::PixelShaderResource);
            break;
        case ResourceReadAccess::NonPixelShader:
            State.Set(RHI::EResourceState::NonPixelShaderResource);
            break;
        case ResourceReadAccess::Any:
            State |= RHI::MResourceState_AllShaderResource;
            break;
        }

        SetResourceState(ViewId, State, Flags);
        m_Storage.DeclareResourceView(ViewId, Desc);

        return ViewId;
    }

    const ResourceViewId& ResourceResolver::ReadSrcResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesRead.emplace(Id);

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(RHI::EResourceState::CopySource), {});

        return ViewId;
    }

    void ResourceResolver::ImportBuffer(
        const ResourceId&        Id,
        const Ptr<RHI::IBuffer>& Resource,
        RHI::GraphicsBufferType  BufferType)
    {
        m_Storage.ImportBuffer(Id, Resource, BufferType);
    }

    void ResourceResolver::ImportTexture(
        const ResourceId&             Id,
        const Ptr<RHI::ITexture>&     Resource,
        const RHI::ClearOperationOpt& ClearValue)
    {
        m_Storage.ImportTexture(Id, Resource, ClearValue);
    }

    RHI::EResourceFormat ResourceResolver::GetSwapchainFormat()
    {
        return RHI::ISwapchain::Get()->GetFormat();
    }

    ResourceId ResourceResolver::GetOutputImageTag()
    {
        return ResourceId(STR("OutputImage"));
    }

    void ResourceResolver::SetResourceState(
        const ResourceViewId&      ViewId,
        const RHI::MResourceState& State,
        const RHI::MResourceFlags& Flags)
    {
        auto& ResourceHandle = m_Storage.GetResourceMut(ViewId.GetResource());
        auto& ResourceDesc   = ResourceHandle.GetDesc();

        ResourceDesc.Flags |= Flags;
        m_ResourceStates[ViewId] |= State;
    }
} // namespace Neon::RG