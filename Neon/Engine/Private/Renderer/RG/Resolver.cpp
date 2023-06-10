#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Resolver.hpp>

namespace Neon::RG
{
    void IRenderPass::ShaderResolver::Load(
        const RHI::ShaderCompileDesc& Desc)
    {
    }

    void IRenderPass::ShaderResolver::Load(
        const Ptr<RHI::IShader>& Shader)
    {
    }

    void IRenderPass::ShaderResolver::Load(
        const Asset::AssetHandle& ShaderAsset)
    {
    }

    //

    void IRenderPass::RootSignature::Load(
        const RHI::RootSignatureBuilder& Builder)
    {
    }

    void IRenderPass::RootSignature::Load(
        const Ptr<RHI::IRootSignature>& RootSignature)
    {
    }

    void IRenderPass::RootSignature::Load(
        const Asset::AssetHandle& RootSigAsset)
    {
    }

    //

    void IRenderPass::PipelineResolver::Load(
        const RHI::PipelineStateBuilder<true>& Builder)
    {
    }

    void IRenderPass::PipelineResolver::Load(
        const RHI::PipelineStateBuilder<false>& Builder)
    {
    }

    void IRenderPass::PipelineResolver::Load(
        const Ptr<RHI::IPipelineState>& PipelineState)
    {
    }

    //

    void IRenderPass::MaterialResolver::Load(
        const Renderer::Material& Material)
    {
    }

    void IRenderPass::MaterialResolver::Load(
        const Asset::AssetHandle& MaterialAsset)
    {
    }

    //

    void IRenderPass::ResourceResolver::CreateBuffer(
        const ResourceId&       Id,
        const ResourceDesc&     Desc,
        RHI::GraphicsBufferType BufferType)
    {
    }

    void IRenderPass::ResourceResolver::CreateTexture(
        const ResourceId&   Id,
        const ResourceDesc& Desc,
        MResourceFlags      Flags)
    {
    }

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