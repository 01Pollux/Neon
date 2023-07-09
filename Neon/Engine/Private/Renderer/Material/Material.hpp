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
            const GenericMaterialBuilder<true>& Builder,
            uint32_t&                           LocalResourceDescriptorSize,
            uint32_t&                           LocalSamplerDescriptorSize);

        Material(
            const GenericMaterialBuilder<false>& Builder,
            uint32_t&                            LocalResourceDescriptorSize,
            uint32_t&                            LocalSamplerDescriptorSize);

        NEON_CLASS_NO_COPYMOVE(Material);

        ~Material() override;

        Ptr<IMaterialInstance> CreateInstance() override;

        const Ptr<IMaterialInstance>& GetDefaultInstance() override;

        void Bind(
            RHI::IGraphicsCommandList* CommandList) override;

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

        Ptr<IMaterialInstance>          m_DefaultInstance;
        std::map<StringU8, LayoutEntry> m_EntryMap;
    };

    class MaterialInstance final : public IMaterialInstance
    {
        friend class Material;
        using DescriptorHeapHandle = RHI::DescriptorHeapHandle;

    public:
        MaterialInstance(
            Ptr<IMaterial> Mat,
            uint32_t       LocaResourceDescriptorSize,
            uint32_t       LocaSamplerDescriptorSize);

        NEON_CLASS_NO_COPYMOVE(MaterialInstance);

        ~MaterialInstance() override;

        [[nodiscard]] Ptr<IMaterialInstance> CreateInstance() override;

        void GetDescriptor(
            RHI::DescriptorHeapHandle* OutResourceDescriptor,
            RHI::DescriptorHeapHandle* OutSamplerDescriptor) const override;

        void SetResource(
            const std::string&             Name,
            const Ptr<RHI::IGpuResource>   Resource,
            const RHI::DescriptorViewDesc& Desc,
            size_t                         ArrayIndex) override;

    private:
        DescriptorHeapHandle
            m_ResourceDescriptor,
            m_SamplerDescriptor;
    };
} // namespace Neon::Renderer