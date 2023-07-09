#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/View.hpp>

namespace Neon::RHI
{
    class ISwapchain;
    class IGraphicsCommandList;
    class IPipelineState;
    class IRootSignature;
    struct DescriptorHeapHandle;
} // namespace Neon::RHI

namespace Neon::Renderer
{
    template<bool>
    class GenericMaterialBuilder;

    class IMaterial
    {
    public:
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
        /// Bind the material to the command list.
        /// </summary>
        virtual void Bind(
            RHI::IGraphicsCommandList* CommandList) = 0;

        /// <summary>
        /// Get the parent material.
        /// </summary>
        [[nodiscard]] virtual void GetDescriptor(
            bool                       Local,
            RHI::DescriptorHeapHandle* OutResourceDescriptor,
            RHI::DescriptorHeapHandle* OutSamplerDescriptor) const = 0;

        /// <summary>
        /// Get the root signature.
        /// </summary>
        virtual const Ptr<RHI::IRootSignature>& GetRootSignature() const = 0;

        /// <summary>
        /// Get pipeline state.
        /// </summary>
        virtual const Ptr<RHI::IPipelineState>& GetPipelineState() const = 0;

    public:
        /// <summary>
        /// Set the material's resource.
        /// </summary>
        virtual void SetResource(
            const std::string&             Name,
            const Ptr<RHI::IGpuResource>   Resource,
            const RHI::DescriptorViewDesc& Desc,
            size_t                         ArrayIndex = 0) = 0;

        /// <summary>
        /// Set the material's resource as a constant buffer view.
        /// </summary>
        void SetConstantBuffer(
            const std::string&           Name,
            const Ptr<RHI::IGpuResource> Resource,
            const RHI::CBVDesc&          Desc,
            size_t                       ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as a resource view.
        /// </summary>
        void SetTexture(
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>       Resource,
            const std::optional<RHI::SRVDesc>& Desc       = std::nullopt,
            size_t                             ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as an unordered access view.
        /// </summary>
        void SetUnorderedAcess(
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>       Resource,
            const std::optional<RHI::UAVDesc>& Desc       = std::nullopt,
            size_t                             ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as a render target view.
        /// </summary>
        void SetRenderTarget(
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>       Resource,
            const std::optional<RHI::RTVDesc>& Desc       = std::nullopt,
            size_t                             ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as a depth stencil view.
        /// </summary>
        void SetDepthStencil(
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>       Resource,
            const std::optional<RHI::DSVDesc>& Desc       = std::nullopt,
            size_t                             ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }
    };
} // namespace Neon::Renderer