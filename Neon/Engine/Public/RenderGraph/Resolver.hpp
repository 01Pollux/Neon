#pragma once

#include <RenderGraph/Pass.hpp>

namespace Neon::RG
{
    class GraphStorage;

    /// <summary>
    /// Purpose of this classes is to defer the loading of resources to the render graph.
    /// This allows to load resources only when they are needed.
    /// </summary>
    class IRenderPass::ResourceResolver
    {
        friend class GraphBuilder;

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
            MResourceFlags           Flags = {});

        /// <summary>
        /// Create window sized texture
        /// </summary>
        void CreateWindowTexture(
            const ResourceId&        Id,
            const RHI::ResourceDesc& Desc,
            MResourceFlags           Flags = {});

    public:
        /// <summary>
        /// Write resource view
        /// </summary>
        ResourceViewId WriteResource(
            const ResourceViewId&          ViewId,
            const RHI::DescriptorViewDesc& Desc);

        /// <summary>
        /// Write resource in copy operation
        /// </summary>
        ResourceViewId WriteDstResource(
            const ResourceViewId& ViewId);

        /// <summary>
        /// Write resource as uav
        /// </summary>
        ResourceViewId WriteResource(
            const ResourceViewId&       ViewId,
            std::optional<RHI::UAVDesc> Desc = std::nullopt)
        {
            return WriteResource(ViewId, RHI::DescriptorViewDesc{ std::move(Desc) });
        }

        /// <summary>
        /// Write resource as renter target
        /// </summary>
        ResourceViewId WriteRenderTarget(
            const ResourceViewId&       ViewId,
            std::optional<RHI::RTVDesc> Desc = std::nullopt)
        {
            return WriteResource(ViewId, RHI::DescriptorViewDesc{ std::move(Desc) });
        }

        /// <summary>
        /// Write resource as renter target
        /// </summary>
        ResourceViewId WriteDepthStencil(
            const ResourceViewId&       ViewId,
            std::optional<RHI::DSVDesc> Desc = std::nullopt)
        {
            return WriteResource(ViewId, RHI::DescriptorViewDesc{ std::move(Desc) });
        }

    public:
        /// <summary>
        /// Read resource view
        /// </summary>
        ResourceViewId ReadResource(
            const ResourceViewId&          ViewId,
            ResourceReadAccess             ReadAccess,
            const RHI::DescriptorViewDesc& Desc);

        /// <summary>
        /// Read resource in copy operation
        /// </summary>
        ResourceViewId ReadSrcResource(
            const ResourceViewId& ViewId);

        /// <summary>
        /// Write resource as uav
        /// </summary>
        ResourceViewId ReadTexture(
            const ResourceViewId&       ViewId,
            ResourceReadAccess          ReadAccess,
            std::optional<RHI::SRVDesc> Desc = std::nullopt)
        {
            return ReadResource(ViewId, ReadAccess, RHI::DescriptorViewDesc{ std::move(Desc) });
        }

        /// <summary>
        /// Write resource as renter target
        /// </summary>
        ResourceViewId ReadDepthStencil(
            const ResourceViewId&       ViewId,
            ResourceReadAccess          ReadAccess,
            std::optional<RHI::DSVDesc> Desc = std::nullopt)
        {
            return ReadResource(ViewId, ReadAccess, RHI::DescriptorViewDesc{ std::move(Desc) });
        }

    public:
        /// <summary>
        /// import buffer to be used later when dispatching passes
        /// </summary>
        void ImportBuffer(
            const ResourceId&        Id,
            const Ptr<RHI::IBuffer>& Resource,
            RHI::GraphicsBufferType  BufferType);

        /// <summary>
        /// import texture to be used later when dispatching passes
        /// </summary>
        void ImportTexture(
            const ResourceId&             Id,
            const Ptr<RHI::ITexture>&     Resource,
            const RHI::ClearOperationOpt& ClearValue = std::nullopt);

        /// <summary>
        /// Get swapchain's format
        /// </summary>
        [[nodiscard]] static RHI::EResourceFormat GetSwapchainFormat();

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