#pragma once

#include <RenderGraph/SceneContext.hpp>
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

        using ResourceMapType = std::map<ResourceId, ResourceHandle>;

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
            const ResourceViewId& ViewId);

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
        /// Get resource size from id
        /// </summary>
        [[nodiscard]] Size2I GetResourceSize(
            const ResourceId& Id) const;

        /// <summary>
        /// Get resource view from id
        /// </summary>
        const RHI::DescriptorViewDesc& GetResourceView(
            ResourceViewId            ViewId,
            RHI::CpuDescriptorHandle* CpuHandle        = nullptr,
            uint32_t*                 SubresourceIndex = nullptr) const;

        /// <summary>
        /// Get resource's view handle from id
        /// </summary>
        [[nodiscard]] RHI::CpuDescriptorHandle GetResourceViewHandle(
            ResourceViewId ViewId) const;

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
        [[nodiscard]] const CameraFrameData& GetFrameData() const;

        /// <summary>
        /// Map global frame data to system memory
        /// </summary>
        [[nodiscard]] CameraFrameData& GetFrameData();

        /// <summary>
        /// Get global frame data
        /// </summary>
        [[nodiscard]] RHI::GpuResourceHandle GetFrameDataHandle() const;

        /// <summary>
        /// Get scene context
        /// </summary>
        [[nodiscard]] const SceneContext& GetSceneContext() const noexcept;

        /// <summary>
        /// Get scene context
        /// </summary>
        [[nodiscard]] SceneContext& GetSceneContext() noexcept;

    public:
        // TODO: Add sampler

        /// <summary>
        /// Declare resource to be created later when dispatching passes
        /// </summary>
        void DeclareResource(
            const ResourceId&        Id,
            const RHI::ResourceDesc& Desc,
            MResourceFlags           Flags);

        /// <summary>
        /// import resource to be used later when dispatching passes
        /// </summary>
        void ImportBuffer(
            const ResourceId&             Id,
            const Ptr<RHI::IGpuResource>& Buffer,
            RHI::GraphicsBufferType       BufferType);

        /// <summary>
        /// import resource to be used later when dispatching passes
        /// </summary>
        void ImportTexture(
            const ResourceId&             Id,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::ClearOperationOpt& ClearValue = std::nullopt);

        /// <summary>
        /// Declare resource view to be created later when dispatching passes
        /// </summary>
        void DeclareResourceView(
            const ResourceViewId&          ViewId,
            const RHI::DescriptorViewDesc& Desc,
            SubresourceView                Subresource);

    private:
        /// <summary>
        /// Allocate a new output image for rendering
        /// </summary>
        void UpdateOutputImage(
            const Size2I& Size);

        /// <summary>
        /// Allocate resource from the pool
        /// </summary>
        void ReallocateResource(
            ResourceHandle& Handle);

    public:
        /// <summary>
        /// Create views in the resource
        /// </summary>
        void CreateViews(
            ResourceHandle& Handle);

    private:
        /// <summary>
        /// Prepare storage for graph dispatching.
        /// </summary>
        void PrepareDispatch();

    private:
        /// <summary>
        /// The graphics buffer of the camera.
        /// Structured as CameraFrameData.
        /// </summary>
        Ptr<RHI::IGpuResource> m_CameraFrameData;
        uint8_t*               m_CameraFrameDataPtr;

        ResourceMapType      m_Resources;
        std::set<ResourceId> m_ImportedResources;
        SceneContext         m_SceneContext;
    };
} // namespace Neon::RG