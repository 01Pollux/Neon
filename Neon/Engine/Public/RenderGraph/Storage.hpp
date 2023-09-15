#pragma once

#include <RenderGraph/Common.hpp>
#include <RenderGraph/CameraFramData.hpp>

#include <deque>
#include <shared_mutex>
#include <set>
#include <map>

namespace Neon::RHI
{
    class IShader;
    class IRootSignature;
    class IPipelineState;
} // namespace Neon::RHI

namespace Neon::RG
{
    class GraphStorage
    {
        friend class RenderGraph;
        friend class GraphDepdencyLevel;

        using ResourceMapType          = std::map<ResourceId, ResourceHandle>;
        using InactiveResourceListType = std::deque<ResourceHandle>;

    public:
        GraphStorage();

        /// <summary>
        /// Reset resources
        /// </summary>
        void Reset();

    public:
        /// <summary>
        /// Check to see if resource of id exists
        /// </summary>
        [[nodiscard]] bool ContainsResource(
            const ResourceId& Id) const;

        /// <summary>
        /// Check to see if resource of id exists
        /// </summary>
        [[nodiscard]] bool ContainsResourceView(
            const ResourceViewId& ViewId) const;

    public:
        /// <summary>
        /// Get resource from id
        /// </summary>
        [[nodiscard]] ResourceHandle& GetResourceMut(
            const ResourceId& Id);

        /// <summary>
        /// Get resource from id
        /// </summary>
        [[nodiscard]] const ResourceHandle& GetResource(
            const ResourceId& Id) const;

        /// <summary>
        /// Get resource view from id
        /// </summary>
        const RHI::DescriptorViewDesc& GetResourceView(
            const ResourceViewId&     ViewId,
            RHI::CpuDescriptorHandle* CpuHandle = nullptr) const;

    public:
        /// <summary>
        /// Get output image
        /// </summary>
        [[nodiscard]] const ResourceHandle& GetOutputImage() const;

        /// <summary>
        /// Get output image size
        /// </summary>
        [[nodiscard]] Size2I GetOutputImageSize() const;

        /// <summary>
        /// Set output image size
        /// If set to std::nullopt, the output image will be the same size as the window
        /// </summary>
        [[nodiscard]] void SetOutputImageSize(
            std::optional<Size2I> Size);

    public:
        /// <summary>
        /// Map global frame data to system memory
        /// </summary>
        [[nodiscard]] CameraFrameData& MapFrameData() const;

        /// <summary>
        /// Unmap global frame data from system memory
        /// </summary>
        void UnmapFrameData() const;

        /// <summary>
        /// Get global frame data
        /// </summary>
        RHI::GpuResourceHandle GetFrameDataHandle() const;

    public:
        // TODO: Add sampler

        /// <summary>
        /// Declare buffer to be created later when dispatching passes
        /// </summary>
        void DeclareBuffer(
            const ResourceId&       Id,
            const RHI::BufferDesc&  Desc,
            RHI::GraphicsBufferType BufferType);

        /// <summary>
        /// Declare texture to be created later when dispatching passes
        /// </summary>
        void DeclareTexture(
            const ResourceId&        Id,
            const RHI::ResourceDesc& Desc,
            MResourceFlags           Flags);

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
        /// Declare resource view to be created later when dispatching passes
        /// </summary>
        void DeclareResourceView(
            const ResourceViewId&          ViewId,
            const RHI::DescriptorViewDesc& Desc);

    private:
        /// <summary>
        /// Allocate a new output image for rendering
        /// </summary>
        void NewOutputImage();

        /// <summary>
        /// Allocate resource from the pool
        /// </summary>
        void ReallocateResource(
            ResourceHandle& Handle);

        /// <summary>
        /// Free resource to the pool
        /// </summary>
        void FreeResource(
            const ResourceHandle& Handle);

        /// <summary>
        /// Free all resource from pool
        /// </summary>
        void FlushResources();

    public:
        /// <summary>
        /// Create views in the resource
        /// </summary>
        void CreateViews(
            ResourceHandle& Handle);

    private:
        /// <summary>
        /// The graphics buffer of the camera.
        /// Structured as CameraFrameData.
        /// </summary>
        Ptr<RHI::IUploadBuffer> m_CameraFrameData;

        ResourceMapType          m_Resources;
        std::set<ResourceId>     m_ImportedResources;
        InactiveResourceListType m_InactiveResources;
    };
} // namespace Neon::RG