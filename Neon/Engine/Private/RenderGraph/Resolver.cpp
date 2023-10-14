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

    void ResourceResolver::CreateResource(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags)
    {
        m_Storage.DeclareResource(Id, Desc, std::move(Flags));
        m_ResourcesCreated.emplace(Id);
    }

    void ResourceResolver::CreateWindowTexture(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags)
    {
        Flags.Set(RG::EResourceFlags::WindowSizeDependent);
        CreateResource(Id, Desc, std::move(Flags));
    }

    //

    void ResourceResolver::WriteResourceEmpty(
        ResourceId Id)
    {
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource '{}' doesn't exists", Id.GetName());
        m_ResourcesWritten.emplace(Id);
    }

    void ResourceResolver::WriteResource(
        const ResourceViewId&          ViewId,
        const RHI::DescriptorViewDesc& Desc,
        const RHI::SubresourceView&    Subresource)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource '{}' doesn't exists", Id.GetName());
        m_Storage.DeclareResourceView(ViewId, Desc, Subresource);

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
                [&State, &Flags, this, ViewId](const RHI::RTVDescOpt& Desc)
                {
                    NEON_ASSERT(!Desc || !std::holds_alternative<std::monostate>(Desc->View), "View must be set");
                    State.Set(RHI::EResourceState::RenderTarget);
                    Flags.Set(RHI::EResourceFlags::AllowRenderTarget);

                    m_RenderTargets.emplace_back(ViewId);
                },
                [&State, &Flags, this, ViewId](const RHI::DSVDescOpt& Desc)
                {
                    NEON_ASSERT(!Desc || !std::holds_alternative<std::monostate>(Desc->View), "View must be set");
                    State.Set(RHI::EResourceState::DepthWrite);
                    Flags.Set(RHI::EResourceFlags::AllowDepthStencil);

                    m_DepthStencil = ViewId;
                },
                [ViewId](const auto&)
                {
                    NEON_ASSERT(false, "Invalid descriptor view type to write to ({})", ViewId.Get());
                } },
            Desc);

        SetResourceState(ViewId, State, Flags);
    }

    void ResourceResolver::WriteDstResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource '{}' doesn't exists", Id.GetName());

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(RHI::EResourceState::CopyDest), {});
        m_ResourcesWritten.emplace(Id);
    }

    //

    void ResourceResolver::ReadResourceEmpty(
        ResourceId Id)
    {
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource '{}' doesn't exists", Id.GetName());
        m_ResourcesRead.emplace(Id);
    }

    void ResourceResolver::ReadResource(
        const ResourceViewId&          ViewId,
        ResourceReadAccess             ReadAccess,
        const RHI::DescriptorViewDesc& Desc,
        const RHI::SubresourceView&    Subresource)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource '{}' doesn't exists", Id.GetName());
        m_Storage.DeclareResourceView(ViewId, Desc, Subresource);

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
                [&State, &Flags, this, ViewId](const RHI::DSVDescOpt& Desc)
                {
                    NEON_ASSERT(!Desc || !std::holds_alternative<std::monostate>(Desc->View), "View must be set");
                    State.Set(RHI::EResourceState::DepthRead);
                    Flags.Set(RHI::EResourceFlags::AllowDepthStencil);

                    m_DepthStencil = ViewId;
                },
                [ViewId](const auto&)
                {
                    NEON_ASSERT(false, "Invalid descriptor view type to read from ({})", ViewId.Get());
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
    }

    void ResourceResolver::ReadSrcResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource '{}' doesn't exists", Id.GetName());

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(RHI::EResourceState::CopySource), {});
        m_ResourcesWritten.emplace(Id);
    }

    //

    void ResourceResolver::ImportBuffer(
        const ResourceId&             Id,
        const Ptr<RHI::IGpuResource>& Resource,
        RHI::GraphicsBufferType       BufferType)
    {
        m_ResourcesCreated.emplace(Id);
        m_Storage.ImportBuffer(Id, Resource, BufferType);
    }

    void ResourceResolver::ImportTexture(
        const ResourceId&             Id,
        const Ptr<RHI::IGpuResource>& Resource,
        const RHI::ClearOperationOpt& ClearValue)
    {
        m_ResourcesCreated.emplace(Id);
        m_Storage.ImportTexture(Id, Resource, ClearValue);
    }

    std::pair<const RHI::ResourceDesc*, MResourceFlags> IRenderPass::ResourceResolver::GetResourceDescAndFlags(
        ResourceId Id) const
    {
        auto& ResourceHandle = m_Storage.GetResource(Id);
        return { &ResourceHandle.GetDesc(), ResourceHandle.GetFlags() };
    }

    //

    RHI::EResourceFormat ResourceResolver::GetSwapchainFormat()
    {
        return RHI::ISwapchain::Get()->GetFormat();
    }

    void ResourceResolver::SetResourceState(
        ResourceViewId             ViewId,
        const RHI::MResourceState& State,
        const RHI::MResourceFlags& Flags)
    {
        auto& ResourceHandle = m_Storage.GetResourceMut(ViewId.GetResource());
        auto& ResourceDesc   = ResourceHandle.GetDesc();

        ResourceDesc.Flags |= Flags;
        auto& States = m_ResourceStates[ViewId];
        States |= State;
    }
} // namespace Neon::RG