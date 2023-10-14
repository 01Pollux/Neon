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
        /// Check to see if material is transparent.
        /// </summary>
        [[nodiscard]] virtual bool IsTransparent() const noexcept = 0;

        /// <summary>
        /// Check to see if material is transparent.
        /// </summary>
        [[nodiscard]] virtual void SetTransparent(
            bool State = true) noexcept = 0;

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
        /// Set the material's resource.
        /// </summary>
        virtual void SetResource(
            const StringU8&                Name,
            const Ptr<RHI::IGpuResource>&  Resource,
            const RHI::DescriptorViewDesc& Desc) = 0;

        /// <summary>
        /// Set the material's sampler.
        /// </summary>
        virtual void SetSampler(
            const StringU8&         Name,
            const RHI::SamplerDesc& Desc) = 0;

        /// <summary>
        /// Set the material's constant.
        /// </summary>
        virtual void SetData(
            const StringU8& Name,
            const void*     Data) = 0;

    public:
        /// <summary>
        /// Set the material's resource as a constant buffer view.
        /// </summary>
        void SetConstantBuffer(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::CBVDesc&           Desc)
        {
            SetResource(Name, Resource, Desc);
        }

        /// <summary>
        /// Set the material's resource as a resource view.
        /// </summary>
        void SetTexture(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::SRVDescOpt&        Desc = std::nullopt)
        {
            SetResource(Name, Resource, Desc);
        }

        /// <summary>
        /// Set the material's resource as a resource view.
        /// </summary>
        void SetStructuredBuffer(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::SRVDescOpt&        Desc = std::nullopt)
        {
            SetTexture(Name, Resource, Desc);
        }

        /// <summary>
        /// Set the material's resource as an unordered access view.
        /// </summary>
        void SetUnorderedAcess(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            const RHI::UAVDescOpt&        Desc = std::nullopt)
        {
            SetResource(Name, Resource, Desc);
        }

    protected:
        PipelineStateList m_PipelineStates;
    };
} // namespace Neon::RHI