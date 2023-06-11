#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Resolver.hpp>
#include <Renderer/RG/Storage.hpp>

#include <RHI/Swapchain.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    void IRenderPass::ShaderResolver::Load(
        const ResourceId&             Id,
        const RHI::ShaderCompileDesc& Desc)
    {
        NEON_ASSERT(m_ShadersToLoad.emplace(Id, Desc).second, "Shader already exists");
    }

    void IRenderPass::ShaderResolver::Load(
        const ResourceId&        Id,
        const Ptr<RHI::IShader>& Shader)
    {
        NEON_ASSERT(m_ShadersToLoad.emplace(Id, Shader).second, "Shader already exists");
    }

    void IRenderPass::ShaderResolver::Load(
        const ResourceId&         Id,
        const Asset::AssetHandle& ShaderAsset)
    {
        NEON_ASSERT(m_ShadersToLoad.emplace(Id, ShaderAsset).second, "Shader already exists");
    }

    //

    void IRenderPass::RootSignatureResolver::Load(
        const ResourceId&                Id,
        const RHI::RootSignatureBuilder& Builder)
    {
        NEON_ASSERT(m_RootSignaturesToLoad.emplace(Id, Builder).second, "Root signature already exists");
    }

    void IRenderPass::RootSignatureResolver::Load(
        const ResourceId&               Id,
        const Ptr<RHI::IRootSignature>& RootSignature)
    {
        NEON_ASSERT(m_RootSignaturesToLoad.emplace(Id, RootSignature).second, "Root signature already exists");
    }

    void IRenderPass::RootSignatureResolver::Load(
        const ResourceId&         Id,
        const Asset::AssetHandle& RootSigAsset)
    {
        NEON_ASSERT(m_RootSignaturesToLoad.emplace(Id, RootSigAsset).second, "Root signature already exists");
    }

    //

    void IRenderPass::PipelineStateResolver::Load(
        const ResourceId&               Id,
        RHI::PipelineStateBuilder<true> Builder)
    {
        NEON_ASSERT(m_PipelinesToLoad.emplace(Id, std::move(Builder)).second, "Pipeline already exists");
    }

    void IRenderPass::PipelineStateResolver::Load(
        const ResourceId&                Id,
        RHI::PipelineStateBuilder<false> Builder)
    {
        NEON_ASSERT(m_PipelinesToLoad.emplace(Id, std::move(Builder)).second, "Pipeline already exists");
    }

    void IRenderPass::PipelineStateResolver::Load(
        const ResourceId&               Id,
        const Ptr<RHI::IPipelineState>& PipelineState)
    {
        NEON_ASSERT(m_PipelinesToLoad.emplace(Id, PipelineState).second, "Pipeline already exists");
    }

    const Ptr<RHI::IRootSignature>& IRenderPass::PipelineStateResolver::GetRootSignature(
        const ResourceId& Id)
    {
        return m_Storage.GetRootSignature(Id);
    }

    const Ptr<RHI::IShader>& IRenderPass::PipelineStateResolver::GetShader(
        const ResourceId& Id)
    {
        return m_Storage.GetShader(Id);
    }

    //

    void IRenderPass::MaterialResolver::Load(
        const ResourceId&         Id,
        const Renderer::Material& Material)
    {
        NEON_ASSERT(m_MaterialsToLoad.emplace(Id, Material).second, "Material already exists");
    }

    void IRenderPass::MaterialResolver::Load(
        const ResourceId&         Id,
        const Asset::AssetHandle& MaterialAsset)
    {
        NEON_ASSERT(m_MaterialsToLoad.emplace(Id, MaterialAsset).second, "Material already exists");
    }

    //

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

    void IRenderPass::ResourceResolver::WriteResource(
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
                [&State, &Flags](const std::optional<RHI::RTVDesc>&)
                {
                    State = RHI::EResourceState::RenderTarget;
                    Flags = RHI::EResourceFlags::AllowRenderTarget;
                },
                [&State, &Flags](const std::optional<RHI::DSVDesc>&)
                {
                    State = RHI::EResourceState::DepthWrite;
                    Flags = RHI::EResourceFlags::AllowDepthStencil;
                },
                [](const auto&)
                {
                    NEON_ASSERT(false, "Invalid descriptor view type to write to");
                } },
            Desc);

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(State), RHI::MResourceFlags::FromEnum(Flags));
        m_Storage.DeclareResourceView(ViewId, Desc);
    }

    void IRenderPass::ResourceResolver::WriteDstResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesWritten.emplace(Id);

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(RHI::EResourceState::CopyDest), {});
    }

    void IRenderPass::ResourceResolver::ReadResource(
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
    }

    void IRenderPass::ResourceResolver::ReadSrcResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT(m_Storage.ContainsResource(Id), "Resource doesn't exists");
        m_ResourcesRead.emplace(Id);

        SetResourceState(ViewId, RHI::MResourceState::FromEnum(RHI::EResourceState::CopySource), {});
    }

    RHI::EResourceFormat IRenderPass::ResourceResolver::GetSwapchainFormat() const
    {
        return m_Storage.GetSwapchain()->GetFormat();
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