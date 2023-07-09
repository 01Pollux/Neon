#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/View.hpp>

namespace Neon::RHI
{
    class ISwapchain;
    class IGraphicsCommandList;
    struct DescriptorHeapHandle;
} // namespace Neon::RHI

namespace Neon::Renderer
{
    class IMaterialInstance;

    template<bool>
    class GenericMaterialBuilder;

    class IMaterial : public std::enable_shared_from_this<IMaterial>
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
        [[nodiscard]] virtual Ptr<IMaterialInstance> CreateInstance() = 0;

        /// <summary>
        /// Get the default material instance.
        /// </summary>
        [[nodiscard]] virtual const Ptr<IMaterialInstance>& GetDefaultInstance() = 0;

        /// <summary>
        /// Bind the material to the command list.
        /// </summary>
        virtual void Bind(
            RHI::IGraphicsCommandList* CommandList) = 0;
    };

    class IMaterialInstance
    {
    public:
        virtual ~IMaterialInstance() = default;

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] virtual Ptr<IMaterialInstance> CreateInstance() = 0;

        /// <summary>
        /// Get the parent material.
        /// </summary>
        [[nodiscard]] const Ptr<IMaterial>& GetParentMaterial() const;

        /// <summary>
        /// Get the parent material.
        /// </summary>
        [[nodiscard]] virtual void GetDescriptor(
            RHI::DescriptorHeapHandle* OutResourceDescriptor,
            RHI::DescriptorHeapHandle* OutSamplerDescriptor) const = 0;

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

    protected:
        Ptr<IMaterial> m_ParentMaterial;
    };
} // namespace Neon::Renderer