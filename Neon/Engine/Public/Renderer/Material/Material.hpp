#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Commands/Common.hpp>

#include <map>
#include <vector>

namespace Neon::RHI
{
    class ISwapchain;
    class ICommandList;
    class IPipelineState;
    class IRootSignature;
} // namespace Neon::RHI

namespace Neon::Renderer
{
    template<bool>
    class GenericMaterialBuilder;

    //

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
        /// Get material's descriptor for a given parameter.
        /// </summary>
        [[nodiscard]] virtual RHI::IDescriptorHeap::CopyInfo GetDescriptorParam(
            const StringU8& ParamName) const = 0;

        /// <summary>
        /// Apply the material to the command list.
        /// </summary>
        virtual void Apply(
            RHI::ICommandList* CommandList) = 0;

        /// <summary>
        /// Get the root signature.
        /// </summary>
        const Ptr<RHI::IRootSignature>& GetRootSignature() const noexcept;

        /// <summary>
        /// Get pipeline state.
        /// </summary>
        const Ptr<RHI::IPipelineState>& GetPipelineState() const noexcept;

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
            const StringU8&                        Name,
            const std::optional<RHI::SamplerDesc>& Desc,
            uint32_t                               ArrayIndex = 0) = 0;

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
        Ptr<RHI::IRootSignature> m_RootSignature;
        Ptr<RHI::IPipelineState> m_PipelineState;
    };

    //

    /// <summary>
    /// Helper class to manage a table of materials for batching.
    /// This class is not thread safe nor does it handle lifetime of the materials.
    /// </summary>
    class MaterialTable
    {
    public:
        /// <summary>
        /// Reset the table.
        /// </summary>
        void Reset();

        /// <summary>
        /// Append a material to the table.
        /// </summary>
        uint32_t Append(
            IMaterial* Material);

        /// <summary>
        /// Get the material at the specified index.
        /// </summary>
        [[nodiscard]] IMaterial* GetFirstMaterial() const
        {
            return m_Materials[0];
        }

        /// <summary>
        /// Bind the material into the command list.
        /// </summary>
        void Apply(
            RHI::ICommandList* CommandList);

        /// <summary>
        /// Bind the material into the command list.
        /// </summary>
        static void ApplyOne(
            IMaterial*         Material,
            RHI::ICommandList* CommandList);

    private:
        /// <summary>
        /// Bind the materials into the command list.
        /// </summary>
        static void Apply(
            IMaterial*            FirstMaterial,
            std::span<IMaterial*> Materials,
            RHI::ICommandList*    CommandList);

    private:
        std::vector<IMaterial*>                  m_Materials;
        std::unordered_map<IMaterial*, uint32_t> m_MaterialMap;
    };
} // namespace Neon::Renderer