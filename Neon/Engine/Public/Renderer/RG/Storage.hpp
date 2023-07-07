#pragma once

#include <Renderer/RG/Common.hpp>
#include <deque>
#include <shared_mutex>
#include <set>
#include <map>

namespace Neon::RHI
{
    class ISwapchain;
    class IShader;
    class IRootSignature;
    class IPipelineState;
} // namespace Neon::RHI

namespace Neon::RG
{
    class GraphStorage
    {
        friend class RenderGraph;
        friend class RenderGraphDepdencyLevel;

        using ResourceMapType       = std::map<ResourceId, ResourceHandle>;
        using ShadersMapType        = std::map<ResourceId, Ptr<RHI::IShader>>;
        using RootSignaturesMapType = std::map<ResourceId, Ptr<RHI::IRootSignature>>;
        using PipelineStatesMapType = std::map<ResourceId, Ptr<RHI::IPipelineState>>;

        using InactiveResourceListType = std::deque<ResourceHandle>;

    public:
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
            RHI::CpuDescriptorHandle* CpuHandle = nullptr,
            RHI::GpuDescriptorHandle* GpuHandle = nullptr) const;

    public:
        /// <summary>
        /// Get root signature
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IRootSignature>& GetRootSignature(
            const ResourceId& Id) const;

        /// <summary>
        /// Get shader
        /// </summary>
        /// <param name="Id"></param>
        /// <returns></returns>
        [[nodiscard]] const Ptr<RHI::IShader>& GetShader(
            const ResourceId& Id) const;

        /// <summary>
        /// Get pipeline state
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IPipelineState>& GetPipelineState(
            const ResourceId& Id) const;

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

    public:
        /// <summary>
        /// Import shader
        /// </summary>
        void ImportShader(
            const ResourceId&        Id,
            const Ptr<RHI::IShader>& Shader);

        /// <summary>
        /// Import root signature
        /// </summary>
        void ImportRootSignature(
            const ResourceId&               Id,
            const Ptr<RHI::IRootSignature>& RootSignature);

        /// <summary>
        /// Import pipeline state
        /// </summary>
        void ImportPipelineState(
            const ResourceId&               Id,
            const Ptr<RHI::IPipelineState>& PipelineState);

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
        template<typename _Ty>
        class CachedRenderResource
        {
        public:
            [[nodiscard]] std::pair<const _Ty*, std::shared_lock<std::shared_mutex>> Read() const
            {
                return { &m_Resource, std::shared_lock(m_Mutex) };
            }

            [[nodiscard]] std::pair<_Ty*, std::unique_lock<std::shared_mutex>> Write()
            {
                return { &m_Resource, std::unique_lock(m_Mutex) };
            }

            void Clear()
            {
                m_Resource.clear();
            }

        private:
            _Ty m_Resource;

            mutable std::shared_mutex m_Mutex;
        };

        ResourceMapType      m_Resources;
        std::set<ResourceId> m_ImportedResources;

        CachedRenderResource<RootSignaturesMapType> m_RootSignatures;
        CachedRenderResource<ShadersMapType>        m_Shaders;
        CachedRenderResource<PipelineStatesMapType> m_PipelineStates;

        InactiveResourceListType m_InactiveResources;
    };
} // namespace Neon::RG