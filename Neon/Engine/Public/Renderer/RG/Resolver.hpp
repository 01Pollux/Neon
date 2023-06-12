#pragma once

#include <Renderer/RG/Pass.hpp>

#include <Resource/Handle.hpp>
#include <Renderer/Material/Material.hpp>

#include <RHI/Shader.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>

namespace Neon
{
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
        friend class RenderGraphBuilder;

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
        friend class RenderGraphBuilder;

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
        friend class RenderGraphBuilder;

    public:
        PipelineStateResolver(
            GraphStorage& Storage) :
            m_Storage(Storage)
        {
        }

        /// <summary>
        /// Load a pipeline state.
        /// </summary>
        void Load(
            const ResourceId&               Id,
            RHI::PipelineStateBuilder<true> Builder);

        /// <summary>
        /// Load a pipeline state.
        /// </summary>
        void Load(
            const ResourceId&                Id,
            RHI::PipelineStateBuilder<false> Builder);

        /// <summary>
        /// Load a pipeline state.
        /// </summary>
        void Load(
            const ResourceId&               Id,
            const Ptr<RHI::IPipelineState>& PipelineState);

        /// <summary>
        /// Get root signature.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IRootSignature>& GetRootSignature(
            const ResourceId& Id);

        /// <summary>
        /// Get shader.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IShader>& GetShader(
            const ResourceId& Id);

    private:
        GraphStorage& m_Storage;

        std::map<
            ResourceId,
            std::variant<
                RHI::PipelineStateBuilder<true>,
                RHI::PipelineStateBuilder<false>,
                Ptr<RHI::IPipelineState>>>
            m_PipelinesToLoad;
    };

    //

    class IRenderPass::ResourceResolver
    {
        friend class RenderGraphBuilder;

    public:
        ResourceResolver(
            GraphStorage& Storage);

        /// <summary>
        /// Create buffer
        /// </summary>
        void CreateBuffer(
            const ResourceId&       Id,
            const RHI::BufferDesc&  Desc,
            RHI::GraphicsBufferType BufferType);

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

        /// <summary>
        /// Get swapchain's format
        /// </summary>
        [[nodiscard]] RHI::EResourceFormat GetSwapchainFormat() const;

    private:
        /// <summary>
        /// Initialize resource to the default state
        /// </summary>
        void SetResourceState(
            const ResourceViewId& ViewId,
            RHI::MResourceState   State,
            RHI::MResourceFlags   Flags);

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