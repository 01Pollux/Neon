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
        /// Create resource
        /// </summary>
        void CreateResource(
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
        /// Write resource view to add dependency without actually writing anything
        /// </summary>
        void WriteResourceEmpty(
            ResourceId Id);

        /// <summary>
        /// Write resource view
        /// RawView indicates that the resource is used as raw buffer and no descriptor will be created
        /// </summary>
        void WriteResource(
            const ResourceViewId&          ViewId,
            const RHI::DescriptorViewDesc& Desc,
            SubresourceView                Subresource = {});

        /// <summary>
        /// Write resource in copy operation
        /// </summary>
        void WriteDstResource(
            const ResourceViewId& ViewId);

        /// <summary>
        /// Write resource as cbv
        /// 0 offset and size means whole buffer
        /// </summary>
        void WriteBuffer(
            const ResourceViewId& ViewId,
            size_t                Offset      = 0,
            size_t                Size        = 0,
            SubresourceView       Subresource = {})
        {
            WriteResource(
                ViewId,
                RHI::DescriptorViewDesc{ RHI::CBVDesc{
                    .Resource = Offset,
                    .Size     = Size } },
                std::move(Subresource));
        }

        /// <summary>
        /// Write resource as uav
        /// </summary>
        void WriteResource(
            const ResourceViewId& ViewId,
            RHI::UAVDescOpt       Desc        = std::nullopt,
            SubresourceView       Subresource = {})
        {
            WriteResource(ViewId, RHI::DescriptorViewDesc{ std::move(Desc) }, std::move(Subresource));
        }

        /// <summary>
        /// Write resource as renter target
        /// </summary>
        void WriteRenderTarget(
            const ResourceViewId& ViewId,
            RHI::RTVDescOpt       Desc        = std::nullopt,
            SubresourceView       Subresource = {})
        {
            WriteResource(ViewId, RHI::DescriptorViewDesc{ std::move(Desc) }, std::move(Subresource));
        }

        /// <summary>
        /// Write resource as renter target
        /// </summary>
        void WriteDepthStencil(
            const ResourceViewId& ViewId,
            RHI::DSVDescOpt       Desc        = std::nullopt,
            SubresourceView       Subresource = {})
        {
            WriteResource(ViewId, RHI::DescriptorViewDesc{ std::move(Desc) }, std::move(Subresource));
        }

    public:
        /// <summary>
        /// Read resource view
        /// </summary>
        void ReadResourceEmpty(
            ResourceId Id);

        /// <summary>
        /// Read resource view
        /// </summary>
        void ReadResource(
            const ResourceViewId&          ViewId,
            ResourceReadAccess             ReadAccess,
            const RHI::DescriptorViewDesc& Desc,
            SubresourceView                Subresource = {});

        /// <summary>
        /// Read resource in copy operation
        /// </summary>
        void ReadSrcResource(
            const ResourceViewId& ViewId);

        /// <summary>
        /// Write resource as srv
        /// </summary>
        void ReadConstantBuffer(
            const ResourceViewId& ViewId,
            ResourceReadAccess    ReadAccess,
            RHI::CBVDesc          Desc,
            SubresourceView       Subresource = {})
        {
            ReadResource(ViewId, ReadAccess, RHI::DescriptorViewDesc{ std::move(Desc) }, std::move(Subresource));
        }

        /// <summary>
        /// Write resource as srv
        /// </summary>
        void ReadTexture(
            const ResourceViewId& ViewId,
            ResourceReadAccess    ReadAccess,
            RHI::SRVDescOpt       Desc        = std::nullopt,
            SubresourceView       Subresource = {})
        {
            ReadResource(ViewId, ReadAccess, RHI::DescriptorViewDesc{ std::move(Desc) }, std::move(Subresource));
        }

        /// <summary>
        /// Write resource as uav
        /// </summary>
        void ReadWriteTexture(
            const ResourceViewId& ViewId,
            ResourceReadAccess    ReadAccess,
            RHI::UAVDescOpt       Desc        = std::nullopt,
            SubresourceView       Subresource = {})
        {
            ReadResource(ViewId, ReadAccess, RHI::DescriptorViewDesc{ std::move(Desc) }, std::move(Subresource));
        }

    public:
        /// <summary>
        /// import buffer to be used later when dispatching passes
        /// </summary>
        void ImportBuffer(
            const ResourceId&             Id,
            const Ptr<RHI::IGpuResource>& Resource,
            RHI::GraphicsBufferType       BufferType);

        /// <summary>
        /// import texture to be used later when dispatching passes
        /// </summary>
        void ImportTexture(
            const ResourceId&             Id,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::ClearOperationOpt& ClearValue = std::nullopt);

        /// <summary>
        /// Get resource desc
        /// </summary>
        [[nodiscard]] std::pair<const RHI::ResourceDesc*, MResourceFlags> GetResourceDescAndFlags(
            ResourceId Id) const;

    public:
        /// <summary>
        /// Get swapchain's format
        /// </summary>
        [[nodiscard]] static RHI::EResourceFormat GetSwapchainFormat();

        /// <summary>
        /// Get output image id
        /// </summary>
        [[nodiscard]] static StringU8 GetOutputImageTag()
        {
            return "OutputImage";
        }

        /// <summary>
        /// Get output image id
        /// </summary>
        [[nodiscard]] static ResourceId GetOutputImage()
        {
            return ResourceId(GetOutputImageTag());
        }

    private:
        /// <summary>
        /// Initialize resource to the default state
        /// </summary>
        void SetResourceState(
            ResourceViewId             ViewId,
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