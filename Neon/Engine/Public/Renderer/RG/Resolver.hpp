#pragma once

#include <Renderer/RG/Pass.hpp>

#include <RHI/Resource/Common.hpp>
#include <RHI/Resource/View.hpp>

#include <RHI/Shader.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>

namespace Neon
{
    namespace Asset
    {
        struct AssetHandle;
    } // namespace Asset
    namespace Renderer
    {
        class Material;
    } // namespace Renderer
} // namespace Neon

/// <summary>
/// Purpose of those classes is to defer the loading of resources to the render graph.
/// This allows to load resources only when they are needed.
/// </summary>
namespace Neon::RG
{
    class GraphStorage;

    class IRenderPass::ShaderResolver
    {
    public:
        /// <summary>
        /// Compile a shader from a compile description.
        /// </summary>
        void Load(
            const ResourceId&             Id,
            const RHI::ShaderCompileDesc& Desc);

        /// <summary>
        /// Load a shader from a compile description.
        /// </summary>
        void Load(
            const ResourceId&        Id,
            const Ptr<RHI::IShader>& Shader);

        /// <summary>
        /// Load a shader from an asset.
        /// </summary>
        void Load(
            const ResourceId&         Id,
            const Asset::AssetHandle& ShaderAsset);

    private:
        std::map<
            ResourceId,
            std::variant<
                RHI::ShaderCompileDesc,
                Ptr<RHI::IShader>,
                Asset::AssetHandle>>
            m_ShadersToLoad;
    };

    //

    class IRenderPass::RootSignatureResolver
    {
    public:
        /// <summary>
        /// Load a root signature.
        /// </summary>
        void Load(
            const ResourceId&                Id,
            const RHI::RootSignatureBuilder& Builder);

        /// <summary>
        /// Load a root signature.
        /// </summary>
        void Load(
            const ResourceId&               Id,
            const Ptr<RHI::IRootSignature>& RootSignature);

        /// <summary>
        /// Load a root signature from an asset.
        /// </summary>
        void Load(
            const ResourceId&         Id,
            const Asset::AssetHandle& RootSigAsset);

    private:
        std::map<
            ResourceId,
            std::variant<
                RHI::RootSignatureBuilder,
                Ptr<RHI::IRootSignature>,
                Asset::AssetHandle>>
            m_RootSignaturesToLoad;
    };

    //

    class IRenderPass::PipelineStateResolver
    {
    public:
        /// <summary>
        /// Load a pipeline state.
        /// </summary>
        void Load(
            const ResourceId&                      Id,
            const RHI::PipelineStateBuilder<true>& Builder);

        /// <summary>
        /// Load a pipeline state.
        /// </summary>
        void Load(
            const ResourceId&                       Id,
            const RHI::PipelineStateBuilder<false>& Builder);

        /// <summary>
        /// Load a pipeline state.
        /// </summary>
        void Load(
            const ResourceId&               Id,
            const Ptr<RHI::IPipelineState>& PipelineState);

    private:
        std::map<
            ResourceId,
            std::variant<
                RHI::PipelineStateBuilder<true>,
                RHI::PipelineStateBuilder<false>,
                Ptr<RHI::IPipelineState>>>
            m_PipelinesToLoad;
    };

    //

    class IRenderPass::MaterialResolver
    {
    public:
        /// <summary>
        /// Load a material.
        /// </summary>
        void Load(
            const ResourceId&         Id,
            const Renderer::Material& Material);

        /// <summary>
        /// Load a material from an asset.
        /// </summary>
        void Load(
            const ResourceId&         Id,
            const Asset::AssetHandle& MaterialAsset);

    private:
        std::map<
            ResourceId,
            std::variant<
                Renderer::Material,
                Asset::AssetHandle>>
            m_MaterialsToLoad;
    };

    //

    class IRenderPass::ResourceResolver
    {
    public:
        ResourceResolver(
            GraphStorage& Storage);

        /// <summary>
        /// Create buffer
        /// </summary>
        void CreateBuffer(
            const ResourceId&        Id,
            const RHI::ResourceDesc& Desc,
            RHI::GraphicsBufferType  BufferType);

        /// <summary>
        /// Create texture
        /// </summary>
        void CreateTexture(
            const ResourceId&        Id,
            const RHI::ResourceDesc& Desc,
            MResourceFlags           Flags);

        /// <summary>
        /// Write resource view
        /// </summary>
        void WriteResource(
            const ResourceViewId&          ViewId,
            const RHI::DescriptorViewDesc& Desc);

        /// <summary>
        /// Write resource in copy operation
        /// </summary>
        void WriteDstResource(
            const ResourceViewId& ViewId);

        /// <summary>
        /// Read resource view
        /// </summary>
        void ReadResource(
            const ResourceViewId&          ViewId,
            ResourceReadAccess             ReadAccess,
            const RHI::DescriptorViewDesc& Desc);

        /// <summary>
        /// Read resource in copy operation
        /// </summary>
        void ReadSrcResource(
            const ResourceViewId& ViewId);

    private:
        GraphStorage& m_Storage;

        std::vector<ResourceViewId>   m_RenderTargets;
        std::optional<ResourceViewId> m_DepthStencil;

        std::set<ResourceId> m_ResourcesCreated;
        std::set<ResourceId> m_ResourcesRead;
        std::set<ResourceId> m_ResourcesWritten;

        std::map<ResourceViewId, RHI::MResourceState> m_ResourceStates;
    };
} // namespace Neon::RG