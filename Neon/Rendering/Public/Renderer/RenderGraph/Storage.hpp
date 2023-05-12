#pragma once

#include <Renderer/RenderGraph/ResourceId.hpp>

namespace RHI
{
    class IDisplayBuffers;
}

namespace Renderer::RG
{
    class RenderGraphStorage
    {
        friend class RenderGraphContext;

        using ResourceMapType      = std::map<ResourceId, ResourceHandle>;
        using RootSignatureMapType = std::map<ResourceId, RHI::RootSignature>;
        using PipelineStateMapType = std::map<ResourceId, RHI::PipelineState>;

        using InactiveResourceListType = std::deque<ResourceHandle>;

    public:
        RenderGraphStorage(
            RHI::IDisplayBuffers* Display);

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
        [[nodiscard]] ResourceHandle& GetResource(
            const ResourceId& Id);

        /// <summary>
        /// Get resource from id
        /// </summary>
        [[nodiscard]] const ResourceHandle& GetResource(
            const ResourceId& Id) const;

        /// <summary>
        /// Get resource viez from id
        /// </summary>
        const ResourceViewDesc& GetResourceView(
            const ResourceViewId&        ViewId,
            D3D12_CPU_DESCRIPTOR_HANDLE* CpuHandle = nullptr,
            D3D12_GPU_DESCRIPTOR_HANDLE* GpuHandle = nullptr) const;

        /// <summary>
        /// Get render display
        /// </summary>
        [[nodiscard]] RHI::IDisplayBuffers* GetDisplay() const;

    public:
        /// <summary>
        /// Get root signature
        /// </summary>
        [[nodiscard]] const RHI::RootSignature& GetRootSignature(
            const ResourceId& Id) const;

        /// <summary>
        /// Get pipeline state
        /// </summary>
        [[nodiscard]] const RHI::PipelineState& GetPipelineState(
            const ResourceId& Id) const;

    public:
        /// <summary>
        /// Declare buffer to be created later when dispatching passes
        /// </summary>
        void DeclareBuffer(
            const ResourceId&       Id,
            const ResourceDesc&     Desc,
            RHI::GraphicsBufferType BufferType);

        /// <summary>
        /// Declare texture to be created later when dispatching passes
        /// </summary>
        void DeclareTexture(
            const ResourceId&   Id,
            const ResourceDesc& Desc,
            ResourceFlags       Flags);

        /// <summary>
        /// import buffer to be used later when dispatching passes
        /// </summary>
        void ImportBuffer(
            const ResourceId&                 Id,
            const RHI::GraphicsResource::Ptr& Resource,
            RHI::GraphicsBufferType           BufferType);

        /// <summary>
        /// import texture to be used later when dispatching passes
        /// </summary>
        void ImportTexture(
            const ResourceId&                 Id,
            const RHI::GraphicsResource::Ptr& Resource,
            const ClearOperationOpt&          ClearValue = std::nullopt);

        /// <summary>
        /// Declare resource view to be created later when dispatching passes
        /// </summary>
        void DeclareResourceView(
            const ResourceViewId&   ViewId,
            const ResourceViewDesc& Desc);

    public:
        /// <summary>
        /// Import root signature
        /// </summary>
        void ImportRootSignature(
            const ResourceId&         Id,
            const RHI::RootSignature& RootSignature);

        /// <summary>
        /// Import pipeline state
        /// </summary>
        void ImportPipelineState(
            const ResourceId&         Id,
            const RHI::PipelineState& PipelineState);

    private:
        /// <summary>
        /// Allocate resource from the pool
        /// </summary>
        void RellocateResource(
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
        RHI::IDisplayBuffers* m_Display;

        ResourceMapType      m_Resources;
        std::set<ResourceId> m_ImportedResources;

        std::mutex           m_PipelineMutex;
        RootSignatureMapType m_RootSignatures;
        PipelineStateMapType m_PipelineStates;

        InactiveResourceListType m_InactiveResources;
    };
} // namespace Renderer::RG