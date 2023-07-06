#pragma once

#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Resource/Resource.hpp>
#include <Renderer/Material/Common.hpp>

namespace Neon::Renderer
{
    class MaterialInstance;

    template<bool>
    class GenericMaterialBuilder;

    class Material : public std::enable_shared_from_this<Material>
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

    public:
        Material(
            RHI::ISwapchain*                    Swapchain,
            const GenericMaterialBuilder<true>& Builder);

        Material(
            RHI::ISwapchain*                     Swapchain,
            const GenericMaterialBuilder<false>& Builder);

        NEON_CLASS_NO_COPYMOVE(Material);

        ~Material();

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] Ptr<MaterialInstance> CreateInstance();

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

    class MaterialInstance
    {
        friend class Material;
        using DescriptorHeapHandle = RHI::DescriptorHeapHandle;

    public:
        MaterialInstance(
            Ptr<Material> Mat,
            uint32_t      LocaResourceDescriptorSize,
            uint32_t      LocaSamplerDescriptorSize);

        NEON_CLASS_NO_COPYMOVE(MaterialInstance);

        ~MaterialInstance();

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] Ptr<MaterialInstance> CreateInstance();

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