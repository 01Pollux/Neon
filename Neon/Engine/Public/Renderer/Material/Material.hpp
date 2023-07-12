#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/Descriptor.hpp>

#include <map>
#include <vector>

namespace Neon::RHI
{
    class ISwapchain;
    class IGraphicsCommandList;
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
        /// Get descriptor table.
        /// </summary>
        [[nodiscard]] virtual void GetDescriptorTable(
            bool                       Local,
            RHI::DescriptorHeapHandle* OutResourceDescriptor,
            RHI::DescriptorHeapHandle* OutSamplerDescriptor) const = 0;

        /// <summary>
        /// Get used descriptors.
        /// </summary>
        [[nodiscard]] virtual void GetUsedDescriptors(
            bool                                         Local,
            std::vector<RHI::IDescriptorHeap::CopyInfo>& OutResourceDescriptors,
            std::vector<RHI::IDescriptorHeap::CopyInfo>& OutSamplerDescriptors) const = 0;

        /// <summary>
        /// Get the root signature.
        /// </summary>
        const Ptr<RHI::IRootSignature>& GetRootSignature() const noexcept;

        /// <summary>
        /// Get pipeline state.
        /// </summary>
        const Ptr<RHI::IPipelineState>& GetPipelineState() const noexcept;

        /// <summary>
        /// Apply all the material's resources.
        /// ResourceDescriptor and SamplerDescriptor are expected to hold the correct descriptors.
        /// They are also expected to be in the correct heap as well as the shared and local descriptors for the material.
        /// </summary>
        virtual void ApplyAll(
            RHI::IGraphicsCommandList*       CommandList,
            const RHI::DescriptorHeapHandle& ResourceDescriptor,
            const RHI::DescriptorHeapHandle& SamplerDescriptor) const = 0;

    public:
        /// <summary>
        /// Set the material's resource.
        /// </summary>
        virtual void SetResource(
            const std::string&             Name,
            const Ptr<RHI::IGpuResource>&  Resource,
            const RHI::DescriptorViewDesc& Desc,
            uint32_t                       ArrayIndex = 0,
            const Ptr<RHI::IGpuResource>&  UavCounter = nullptr) = 0;

        /// <summary>
        /// Set the material's resource as a sampler.
        /// </summary>
        virtual void SetResource(
            const std::string&      Name,
            const RHI::SamplerDesc& Desc,
            uint32_t                ArrayIndex = 0) = 0;

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
            const std::string&            Name,
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
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>&      Resource,
            const std::optional<RHI::SRVDesc>& Desc       = std::nullopt,
            uint32_t                           ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as a resource view.
        /// </summary>
        void SetStructuredBuffer(
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>&      Resource,
            const std::optional<RHI::SRVDesc>& Desc       = std::nullopt,
            uint32_t                           ArrayIndex = 0)
        {
            SetTexture(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as an unordered access view.
        /// </summary>
        void SetUnorderedAcess(
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>&      Resource,
            const std::optional<RHI::UAVDesc>& Desc       = std::nullopt,
            uint32_t                           ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as an unordered access view.
        /// </summary>
        void SetUnorderedAcess(
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>&      Resource,
            const Ptr<RHI::IGpuResource>&      Counter,
            const std::optional<RHI::UAVDesc>& Desc       = std::nullopt,
            uint32_t                           ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex, Counter);
        }

        /// <summary>
        /// Set the material's resource as a render target view.
        /// </summary>
        void SetRenderTarget(
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>&      Resource,
            const std::optional<RHI::RTVDesc>& Desc       = std::nullopt,
            uint32_t                           ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

        /// <summary>
        /// Set the material's resource as a depth stencil view.
        /// </summary>
        void SetDepthStencil(
            const std::string&                 Name,
            const Ptr<RHI::IGpuResource>&      Resource,
            const std::optional<RHI::DSVDesc>& Desc       = std::nullopt,
            uint32_t                           ArrayIndex = 0)
        {
            SetResource(Name, Resource, Desc, ArrayIndex);
        }

    protected:
        Ptr<RHI::IRootSignature> m_RootSignature;
        Ptr<RHI::IPipelineState> m_PipelineState;
    };

    //

    /// <summary>
    /// Helper class to bind a material to a command list.
    /// All materials must share the same root signature and pipeline state.
    /// This class is not thread safe nor does it handle lifetime of the materials.
    /// </summary>
    class MaterialBinder
    {
    public:
        MaterialBinder(
            std::span<IMaterial*> Materials);

        /// <summary>
        /// Bind the material's pipeline state and root signature to the command list.
        /// </summary>
        void Bind(
            RHI::IGraphicsCommandList* CommandList);

        /// <summary>
        /// Bind the material's parameters to the command list.
        /// </summary>
        void BindParams(
            RHI::IGraphicsCommandList* CommandList);

        /// <summary>
        /// Bind all the material's resources to the command list as well as the pipeline state and root signature.
        /// </summary>
        void BindAll(
            RHI::IGraphicsCommandList* CommandList);

    private:
        std::span<IMaterial*> m_Materials;
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
        [[nodiscard]] int Append(
            IMaterial* Material);

        /// <summary>
        /// Get the material at the specified index.
        /// </summary>
        [[nodiscard]] IMaterial* GetMaterial(
            size_t Index) const
        {
            return m_Materials[Index];
        }

        /// <summary>
        /// Get the number of materials.
        /// </summary>
        [[nodiscard]] uint32_t GetMaterialCount() const noexcept
        {
            return uint32_t(m_Materials.size());
        }

        /// <summary>
        /// Get the materials.
        /// </summary>
        [[nodiscard]] const auto& GetMaterials() const noexcept
        {
            return m_Materials;
        }

        /// <summary>
        /// Get the materials.
        /// </summary>
        [[nodiscard]] auto& GetMaterials() noexcept
        {
            return m_Materials;
        }

    private:
        std::vector<IMaterial*> m_Materials;
    };

} // namespace Neon::Renderer