#pragma once

#include <Renderer/Material/Material.hpp>
#include <Renderer/Material/Common.hpp>

#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Resource/Resource.hpp>

namespace Neon::Renderer
{
    class MaterialInstance;

    template<bool>
    class GenericMaterialBuilder;

    class Material final : public IMaterial
    {
        static constexpr uint32_t UnboundedTableSize = 0x7FFF;

        friend class MaterialInstance;
        using DescriptorHeapHandle = RHI::DescriptorHeapHandle;

        template<bool _Compute>
        friend void Material_CreateDescriptors(
            RHI::ISwapchain*                        Swapchain,
            const GenericMaterialBuilder<_Compute>& Builder,
            Material*                               Mat,
            uint32_t&                               LocaResourceDescriptorSize,
            uint32_t&                               LocaSamplerDescriptorSize);

        template<bool _Compute>
        friend void Material_CreatePipelineState(
            const GenericMaterialBuilder<_Compute>& Builder,
            Material*                               Mat);

    public:
        Material(
            RHI::ISwapchain*                    Swapchain,
            const GenericMaterialBuilder<true>& Builder,
            uint32_t&                           LocalResourceDescriptorSize,
            uint32_t&                           LocalSamplerDescriptorSize);

        Material(
            RHI::ISwapchain*                     Swapchain,
            const GenericMaterialBuilder<false>& Builder,
            uint32_t&                            LocalResourceDescriptorSize,
            uint32_t&                            LocalSamplerDescriptorSize);

        NEON_CLASS_NO_COPYMOVE(Material);

        ~Material() override;

        Ptr<MaterialInstance> CreateInstance() override;

    public:
        /// <summary>
        /// Create a default material instance.
        /// </summary>
        void CreateDefaultInstance(
            uint32_t LocalResourceDescriptorSize,
            uint32_t LocalSamplerDescriptorSize);

    private:
        struct ConstantEntry
        {
            std::vector<uint8_t> Data;
            size_t               DataStride;
        };

        struct DescriptorEntry
        {
            std::vector<RHI::DescriptorViewDesc> Descs;
            std::vector<Ptr<RHI::IGpuResource>>  Resources;

            uint32_t        Offset;
            MaterialVarType Type;
        };

        struct SamplerEntry
        {
            std::vector<RHI::SamplerDesc> Descs;

            uint32_t Offset;
            uint32_t Count;
        };

        using EntryVariant = std::variant<DescriptorEntry, ConstantEntry, SamplerEntry>;

        struct LayoutEntry
        {
            EntryVariant Entry;
            uint32_t     RootIndex;
            bool         IsShared;
        };

    private:
        RHI::ISwapchain* m_Swapchain;

        Ptr<RHI::IRootSignature> m_RootSignature;
        Ptr<RHI::IPipelineState> m_PipelineState;

        DescriptorHeapHandle
            m_SharedResourceDescriptor,
            m_SharedSamplerDescriptor;

        Ptr<MaterialInstance>           m_DefaultInstace;
        std::map<StringU8, LayoutEntry> m_EntryMap;
    };

    class MaterialInstance final : public IMaterialInstance
    {
        friend class Material;
        using DescriptorHeapHandle = RHI::DescriptorHeapHandle;

    public:
        MaterialInstance(
            Ptr<Material> Mat,
            uint32_t      LocaResourceDescriptorSize,
            uint32_t      LocaSamplerDescriptorSize);

        NEON_CLASS_NO_COPYMOVE(MaterialInstance);

        ~MaterialInstance() override;

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] Ptr<MaterialInstance> CreateInstance() override;

        /// <summary>
        /// Get the parent material.
        /// </summary>
        [[nodiscard]] const Ptr<Material>& GetParentMaterial() const;

    private:
        RHI::ISwapchain* m_Swapchain;
        Ptr<Material>    m_ParentMaterial;

        DescriptorHeapHandle
            m_ResourceDescriptor,
            m_SamplerDescriptor;
    };
} // namespace Neon::Renderer