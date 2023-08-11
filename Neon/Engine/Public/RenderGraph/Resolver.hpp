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

    public:
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
        /// RawView indicates that the resource is used as raw buffer and no descriptor will be created
        /// </summary>
        const ResourceViewId& WriteResource(
            const ResourceViewId&          ViewId,
            const RHI::DescriptorViewDesc& Desc);

        /// <summary>
        /// Write resource in copy operation
        /// </summary>
        const ResourceViewId& WriteDstResource(
            const ResourceViewId& ViewId);

        /// <summary>
        /// Write resource as cbv
        /// 0 offset and size means whole buffer
        /// </summary>
        const ResourceViewId& WriteBuffer(
            const ResourceViewId& ViewId,
            size_t                Offset = 0,
            size_t                Size   = 0)
        {
            return WriteResource(ViewId, RHI::DescriptorViewDesc{ RHI::CBVDesc{
                                             .Resource = Offset,
                                             .Size     = Size } });
        }

        /// <summary>
        /// Write resource as uav
        /// </summary>
        const ResourceViewId& WriteResource(
            const ResourceViewId&       ViewId,
            std::optional<RHI::UAVDesc> Desc = std::nullopt)
        {
            return WriteResource(ViewId, RHI::DescriptorViewDesc{ std::move(Desc) });
        }

        /// <summary>
        /// Write resource as renter target
        /// </summary>
        const ResourceViewId& WriteRenderTarget(
            const ResourceViewId&       ViewId,
            std::optional<RHI::RTVDesc> Desc = std::nullopt)
        {
            return WriteResource(ViewId, RHI::DescriptorViewDesc{ std::move(Desc) });
        }

        /// <summary>
        /// Write resource as renter target
        /// </summary>
        const ResourceViewId& WriteDepthStencil(
            const ResourceViewId&       ViewId,
            std::optional<RHI::DSVDesc> Desc = std::nullopt)
        {
            return WriteResource(ViewId, RHI::DescriptorViewDesc{ std::move(Desc) });
        }

    public:
        /// <summary>
        /// Read resource view
        /// </summary>
        const ResourceViewId& ReadResource(
            const ResourceViewId&          ViewId,
            ResourceReadAccess             ReadAccess,
            const RHI::DescriptorViewDesc& Desc);

        /// <summary>
        /// Read resource in copy operation
        /// </summary>
        const ResourceViewId& ReadSrcResource(
            const ResourceViewId& ViewId);

        /// <summary>
        /// Write resource as uav
        /// </summary>
        const ResourceViewId& ReadTexture(
            const ResourceViewId&       ViewId,
            ResourceReadAccess          ReadAccess,
            std::optional<RHI::SRVDesc> Desc = std::nullopt)
        {
            return ReadResource(ViewId, ReadAccess, RHI::DescriptorViewDesc{ std::move(Desc) });
        }

        /// <summary>
        /// Write resource as renter target
        /// </summary>
        const ResourceViewId& ReadDepthStencil(
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

        /// <summary>
        /// Get output image id
        /// </summary>
        [[nodiscard]] static ResourceId GetOutputImage();

    private:
        /// <summary>
        /// Initialize resource to the default state
        /// </summary>
        void SetResourceState(
            const ResourceViewId&      ViewId,
            const RHI::MResourceState& State,
            const RHI::MResourceFlags& Flags);

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