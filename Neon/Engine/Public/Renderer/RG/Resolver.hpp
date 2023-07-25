#pragma once

#include <Renderer/RG/Pass.hpp>

#include <Asset/Asset.hpp>

#include <RHI/Shader.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>

/// <summary>
/// Purpose of those classes is to defer the loading of resources to the render graph.
/// This allows to load resources only when they are needed.
/// </summary>
namespace Neon::RG
{
    class GraphStorage;

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