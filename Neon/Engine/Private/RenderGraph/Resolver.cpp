#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Resolver.hpp>
#include <RenderGraph/Storage.hpp>

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

    void IRenderPass::ResourceResolver::CreateWindowTexture(
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        MResourceFlags           Flags)
    {
        Flags.Set(RG::EResourceFlags::WindowSizeDependent);
        CreateTexture(Id, Desc, std::move(Flags));
    }

    const ResourceViewId& IRenderPass::ResourceResolver::WriteResource(
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
                [&State, &Flags](const std::optional<RHI::UAVDesc>&)
                {
                    State.Set(RHI::EResourceState::UnorderedAccess);
                    Flags.Set(RHI::EResourceFlags::AllowUnorderedAccess);
                },
                [&State, &Flags, this, &ViewId](const std::optional<RHI::RTVDesc>&)
                {
                    State.Set(RHI::EResourceState::RenderTarget);
                    Flags.Set(RHI::EResourceFlags::AllowRenderTarget);

                    m_RenderTargets.emplace_back(ViewId);
                },
                [&State, &Flags, this, &ViewId](const std::optional<RHI::DSVDesc>&)
                {
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

    const ResourceViewId& IRenderPass::ResourceResolver::WriteDstResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesWritten.emplace(Id);

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(RHI::EResourceState::CopyDest), {});

        return ViewId;
    }

    const ResourceViewId& IRenderPass::ResourceResolver::ReadResource(
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
                [&State, &Flags](const std::optional<RHI::SRVDesc>&) {},
                [&State, &Flags](const std::optional<RHI::UAVDesc>&)
                {
                    State.Set(RHI::EResourceState::UnorderedAccess);
                    Flags.Set(RHI::EResourceFlags::AllowUnorderedAccess);
                },
                [&State, &Flags, this, &ViewId](const std::optional<RHI::DSVDesc>&)
                {
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

    const ResourceViewId& IRenderPass::ResourceResolver::ReadSrcResource(
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

    ResourceId IRenderPass::ResourceResolver::GetOutputImage()
    {
        return ResourceId(STR("OutputImage"));
    }

    void IRenderPass::ResourceResolver::SetResourceState(
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