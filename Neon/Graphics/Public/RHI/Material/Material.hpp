#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Commands/Common.hpp>

#include <map>
#include <vector>

namespace Neon::RHI
{
    class ICommandList;
    class IPipelineState;
    class IRootSignature;

    template<bool>
    class GenericMaterialBuilder;

    //

    class IMaterial
    {
    public:
        enum class PipelineVariant : uint8_t
        {
            /// <summary>
            /// Render pass.
            /// Used for forward+ rendering.
            /// </summary>
            RenderPass,

            /// <summary>
            /// Compute pass.
            /// </summary>
            ComputePass = RenderPass,

            /// <summary>
            /// Render pass.
            /// Used for forward+ rendering.
            /// </summary>
            RenderPassTransparent,

            /// <summary>
            /// Depth prepass.
            /// Used for forward+ rendering.
            /// </summary>
            DepthPrePass,

            Count
        };

        using PipelineStateList = std::array<Ptr<IPipelineState>, size_t(PipelineVariant::Count)>;

        static Ptr<IMaterial> Create(
            const GenericMaterialBuilder<false>& Builder);

        static Ptr<IMaterial> Create(
            const GenericMaterialBuilder<true>& Builder);

        virtual ~IMaterial() = default;

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] virtual Ptr<IMaterial> CreateInstance() = 0;

        /// <summary>
        /// Check to see if material is implemented in compute shader.
        /// </summary>
        [[nodiscard]] virtual bool IsCompute() const noexcept = 0;

        /// <summary>
        /// Get the root signature.
        /// </summary>
        const Ptr<IRootSignature>& GetRootSignature() const noexcept
        {
            return m_RootSignature;
        }

        /// <summary>
        /// Get pipeline state.
        /// </summary>
        const Ptr<IPipelineState>& GetPipelineState(
            PipelineVariant Variant) const noexcept
        {
            return m_PipelineStates[static_cast<uint8_t>(Variant)];
        }

    public:
        /// <summary>
        /// Bind shared parameters directly into the command list.
        /// </summary>
        virtual void BindSharedParams(
            ICommandList* CommandList) = 0;

        /// <summary>
        /// Bind local parameters directly into the command list.
        /// </summary>
        virtual void BindLocalParams(
            ICommandList* CommandList) = 0;

    public:
        /// <summary>
        /// Set the material's resource.
        /// </summary>
        virtual void SetResource(
            const StringU8&                Name,
            const Ptr<RHI::IGpuResource>&  Resource,
            const RHI::DescriptorViewDesc& Desc,
            uint32_t                       ArrayIndex = 0,
            const Ptr<RHI::IGpuResource>&  Counter    = nullptr) = 0;

        /// <summary>
        /// Set the material's resource as a sampler.
        /// </summary>
        virtual void SetSampler(
            const StringU8&         Name,
            const RHI::SamplerDesc& Desc,
            uint32_t                ArrayIndex = 0) = 0;

        /// <summary>
        /// Set the material's resource as a sampler.
        /// </summary>
        virtual void SetResourceView(
            const StringU8&        Name,
            RHI::GpuResourceHandle Handle) = 0;

        /// <summary>
        /// Set the material's resource as a sampler.
        /// </summary>
        virtual void SetDynamicResourceView(
            const StringU8&          Name,
            RHI::CstResourceViewType Type,
            const void*              Data,
            size_t                   Size) = 0;

        /// <summary>
        /// Set the material's resource as a constant.
        /// </summary>
        virtual void SetConstant(
            const StringU8& Name,
            const void*     Data,
            size_t          Size,
            uint32_t        Offset = 0) = 0;

        /// <summary>
        /// Set the material's resource size.
        /// The materials mustn't be instanced nor have a bounded array size.
        /// </summary>
        virtual void SetResourceSize(
            const StringU8& Name,
            uint32_t        Size) = 0;

        //

        /// <summary>
        /// Set the material's resource as a constant buffer view.
        /// </summary>
        void SetConstantBuffer(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::CBVDesc&           Desc,
            uint32_t                      ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as a resource view.
        /// </summary>
        void SetTexture(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::SRVDescOpt&        Desc       = std::nullopt,
            uint32_t                      ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as a resource view.
        /// </summary>
        void SetStructuredBuffer(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::SRVDescOpt&        Desc       = std::nullopt,
            uint32_t                      ArrayIndex = 0)
        {
            SetTexture(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as an unordered access view.
        /// </summary>
        void SetUnorderedAcess(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::UAVDescOpt&        Desc       = std::nullopt,
            uint32_t                      ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as an unordered access view.
        /// </summary>
        void SetUnorderedAcess(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const Ptr<RHI::IGpuResource>& Counter,
            const RHI::UAVDescOpt&        Desc       = std::nullopt,
            uint32_t                      ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex, Counter);
        }

        /// <summary>
        /// Set the material's resource as a render target view.
        /// </summary>
        void SetRenderTarget(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::RTVDescOpt&        Desc       = std::nullopt,
            uint32_t                      ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as a depth stencil view.
        /// </summary>
        void SetDepthStencil(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::DSVDescOpt&        Desc       = std::nullopt,
            uint32_t                      ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

    protected:
        Ptr<IRootSignature> m_RootSignature;
        PipelineStateList   m_PipelineStates;
    };
} // namespace Neon::RHI