#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Resolver.hpp>
#include <Renderer/RG/Storage.hpp>

#include <RHI/Swapchain.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    IRenderPass::ResourceResolver::ResourceResolver(
        GraphStorage& Storage) :
        m_Storage(Storage)
    {
    }

    void IRenderPass::ResourceResolver::CreateBuffer(
        const ResourceId&       Id,
        const RHI::BufferDesc&  Desc,
        RHI::GraphicsBufferType BufferType)
    {
        m_Storage.DeclareBuffer(Id, Desc, BufferType);
        m_ResourcesCreated.emplace(Id);
    }

    void IRenderPass::ResourceResolver::CreateTexture(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags)
    {
        m_Storage.DeclareTexture(Id, Desc, std::move(Flags));
        m_ResourcesCreated.emplace(Id);
    }

    ResourceViewId IRenderPass::ResourceResolver::WriteResource(
        const ResourceViewId&          ViewId,
        const RHI::DescriptorViewDesc& Desc)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesWritten.emplace(Id);

        RHI::EResourceState State{};
        RHI::EResourceFlags Flags{};

        std::visit(
            VariantVisitor{
                [&State, &Flags](const std::optional<RHI::UAVDesc>&)
                {
                    State = RHI::EResourceState::UnorderedAccess;
                    Flags = RHI::EResourceFlags::AllowUnorderedAccess;
                },
                [&State, &Flags, this, &ViewId](const std::optional<RHI::RTVDesc>&)
                {
                    State = RHI::EResourceState::RenderTarget;
                    Flags = RHI::EResourceFlags::AllowRenderTarget;

                    m_RenderTargets.emplace_back(ViewId);
                },
                [&State, &Flags, this, &ViewId](const std::optional<RHI::DSVDesc>&)
                {
                    State = RHI::EResourceState::DepthWrite;
                    Flags = RHI::EResourceFlags::AllowDepthStencil;

                    m_DepthStencil = ViewId;
                },
                [](const auto&)
                {
                    NEON_ASSERT(false, "Invalid descriptor view type to write to");
                } },
            Desc);

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(State), RHI::MResourceFlags::FromEnum(Flags));
        m_Storage.DeclareResourceView(ViewId, Desc);

        return ViewId;
    }

    ResourceViewId IRenderPass::ResourceResolver::WriteDstResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesWritten.emplace(Id);

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(RHI::EResourceState::CopyDest), {});

        return ViewId;
    }

    ResourceViewId IRenderPass::ResourceResolver::ReadResource(
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
                [&State, &Flags](const std::optional<RHI::CBVDesc>&)
                {
                    State = RHI::MResourceState::FromEnum(RHI::EResourceState::ConstantBuffer);
                },
                [&State, &Flags](const std::optional<RHI::SRVDesc>&) {},
                [&State, &Flags](const std::optional<RHI::UAVDesc>&)
                {
                    State = RHI::MResourceState::FromEnum(RHI::EResourceState::UnorderedAccess);
                    Flags = RHI::MResourceFlags::FromEnum(RHI::EResourceFlags::AllowUnorderedAccess);
                },
                [](const auto&)
                {
                    NEON_ASSERT(false, "Invalid descriptor view type to read from");
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

    ResourceViewId IRenderPass::ResourceResolver::ReadSrcResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesRead.emplace(Id);

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(RHI::EResourceState::CopySource), {});

        return ViewId;
    }

    void IRenderPass::ResourceResolver::ImportBuffer(
        const ResourceId&        Id,
        const Ptr<RHI::IBuffer>& Resource,
        RHI::GraphicsBufferType  BufferType)
    {
        m_Storage.ImportBuffer(Id, Resource, BufferType);
    }

    void IRenderPass::ResourceResolver::ImportTexture(
        const ResourceId&             Id,
        const Ptr<RHI::ITexture>&     Resource,
        const RHI::ClearOperationOpt& ClearValue)
    {
        m_Storage.ImportTexture(Id, Resource, ClearValue);
    }

    RHI::EResourceFormat IRenderPass::ResourceResolver::GetSwapchainFormat()
    {
        return RHI::ISwapchain::Get()->GetFormat();
    }

    void IRenderPass::ResourceResolver::SetResourceState(
        const ResourceViewId& ViewId,
        RHI::MResourceState   State,
        RHI::MResourceFlags   Flags)
    {
        auto& ResourceHandle = m_Storage.GetResourceMut(ViewId.GetResource());
        auto& ResourceDesc   = ResourceHandle.GetDesc();

        ResourceDesc.Flags |= Flags;
        m_ResourceStates[ViewId] |= State;
    }
} // namespace Neon::RG