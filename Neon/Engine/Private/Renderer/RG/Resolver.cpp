#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Resolver.hpp>
#include <Renderer/RG/Storage.hpp>

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
        const ResourceId&                      Id,
        const RHI::PipelineStateBuilder<true>& Builder)
    {
        NEON_ASSERT(m_PipelinesToLoad.emplace(Id, Builder).second, "Pipeline already exists");
    }

    void IRenderPass::PipelineStateResolver::Load(
        const ResourceId&                       Id,
        const RHI::PipelineStateBuilder<false>& Builder)
    {
        NEON_ASSERT(m_PipelinesToLoad.emplace(Id, Builder).second, "Pipeline already exists");
    }

    void IRenderPass::PipelineStateResolver::Load(
        const ResourceId&               Id,
        const Ptr<RHI::IPipelineState>& PipelineState)
    {
        NEON_ASSERT(m_PipelinesToLoad.emplace(Id, PipelineState).second, "Pipeline already exists");
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
        const ResourceId&        Id,
        const RHI::ResourceDesc& Desc,
        RHI::GraphicsBufferType  BufferType)
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

    // TODO

    void IRenderPass::ResourceResolver::WriteResource(
        const ResourceViewId&          ViewId,
        const RHI::DescriptorViewDesc& Desc)
    {
    }

    void IRenderPass::ResourceResolver::WriteDstResource(
        const ResourceViewId& ViewId)
    {
    }

    void IRenderPass::ResourceResolver::ReadResource(
        const ResourceViewId&          ViewId,
        ResourceReadAccess             ReadAccess,
        const RHI::DescriptorViewDesc& Desc)
    {
    }

    void IRenderPass::ResourceResolver::ReadSrcResource(
        const ResourceViewId& ViewId)
    {
    }
} // namespace Neon::RG