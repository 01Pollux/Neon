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
            Material*                               Mat);

        template<bool _Compute>
        friend void Material_CreatePipelineState(
            const GenericMaterialBuilder<_Compute>& Builder,
            Material*                               Mat);

    public:
        Material(
            const GenericMaterialBuilder<true>& Builder);

        Material(
            const GenericMaterialBuilder<false>& Builder);

        Material(
            Material* Other);

        NEON_CLASS_NO_COPYMOVE(Material);

        ~Material() override = default;

        Ptr<IMaterial> CreateInstance() override;

        void GetDescriptorTable(
            bool                       Local,
            RHI::DescriptorHeapHandle* OutResourceDescriptor,
            RHI::DescriptorHeapHandle* OutSamplerDescriptor) const override;

        void GetUsedDescriptors(
            bool                                         Local,
            std::vector<RHI::IDescriptorHeap::CopyInfo>& OutResourceDescriptor,
            std::vector<RHI::IDescriptorHeap::CopyInfo>& OutSamplerDescriptor) const override;

        void ApplyAll(
            RHI::IGraphicsCommandList*       CommandList,
            const RHI::DescriptorHeapHandle& ResourceDescriptor,
            const RHI::DescriptorHeapHandle& SamplerDescriptor) const override;

    public:
        void SetResource(
            const StringU8&                Name,
            const Ptr<RHI::IGpuResource>&  Resource,
            const RHI::DescriptorViewDesc& Desc,
            uint32_t                       ArrayIndex,
            const Ptr<RHI::IGpuResource>&  UavCounter) override;

        void SetResource(
            const StringU8&         Name,
            const RHI::SamplerDesc& Desc,
            uint32_t                ArrayIndex = 0) override;

        void SetResourceSize(
            const StringU8& Name,
            uint32_t        Size) override;

    private:
        struct ConstantEntry
        {
            std::vector<uint8_t> Data;
            struct Root
            {
                uint8_t* Data;
                size_t   Num32Bits;
            };
        };

        struct DescriptorEntry
        {
            std::map<uint32_t, Ptr<RHI::IGpuResource>> Resources;

            uint32_t Offset;
            uint32_t Count;

            MaterialVarType Type;

            struct Root
            {
                uint32_t Offset;
            };
        };

        struct SamplerEntry
        {
            std::vector<RHI::SamplerDesc> Descs;

            uint32_t Offset;
            uint32_t Count;

            struct Root
            {
                uint32_t Offset;
            };
        };

        using EntryVariant     = std::variant<ConstantEntry, DescriptorEntry, SamplerEntry>;
        using RootEntryVariant = std::variant<ConstantEntry::Root, DescriptorEntry::Root, SamplerEntry::Root>;

        struct LayoutEntry
        {
            EntryVariant Entry;
            bool         IsInstanced;
        };

        using LayoutEntryMap = std::map<StringU8, LayoutEntry>;

        struct MaterialDescriptor
        {
            LayoutEntryMap                Entries;
            std::vector<RootEntryVariant> RootParams;
        };

    private:
        struct UnqiueDescriptorHeapHandle
        {
            DescriptorHeapHandle ResourceDescriptors, SamplerDescriptors;

            UnqiueDescriptorHeapHandle() = default;
            UnqiueDescriptorHeapHandle(
                uint32_t ResourceDescriptorSize,
                uint32_t SamplerDescriptorSize);

            NEON_CLASS_NO_COPY(UnqiueDescriptorHeapHandle);
            NEON_CLASS_MOVE_DECL(UnqiueDescriptorHeapHandle);

            ~UnqiueDescriptorHeapHandle();
        };

    private:
        Ptr<UnqiueDescriptorHeapHandle> m_SharedDescriptors;
        UnqiueDescriptorHeapHandle      m_LocalDescriptors;

        Ptr<MaterialDescriptor> m_Descriptor;
    };
} // namespace Neon::Renderer