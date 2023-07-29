#pragma once

#include <Renderer/Material/Material.hpp>
#include <Renderer/Material/Common.hpp>

#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Resource/Resource.hpp>

#include <RHI/Commands/List.hpp>
#include <RHI/RootSignature.hpp>

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

        RHI::IDescriptorHeap::CopyInfo GetDescriptorParam(
            const StringU8& ParamName) const override;

        /// <summary>
        /// Apply all the material's resources.
        /// ResourceDescriptor and SamplerDescriptor are expected to hold the correct descriptors.
        /// They are also expected to be in the correct heap as well as the shared and local descriptors for the material.
        /// </summary>
        /// <param name="DescriptorOffsets">
        /// Offset of the descriptor in the heap, contains offsets of resource and sampler descriptors combined and ordered by root parameter index.
        /// </param>
        void ApplyAll(
            RHI::ICommonCommandList*         CommandList,
            std::span<uint32_t>              DescriptorOffsets,
            const RHI::DescriptorHeapHandle& ResourceDescriptor,
            const RHI::DescriptorHeapHandle& SamplerDescriptor) const;

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

        void SetConstant(
            const std::string& Name,
            const void*        Data,
            size_t             Size,
            uint32_t           Offset);

        void SetResourceView(
            const std::string&            Name,
            const Ptr<RHI::IGpuResource>& Resource);

        void SetResourceSize(
            const StringU8& Name,
            uint32_t        Size) override;

    private:
        struct ConstantEntry
        {
            uint8_t DataOffset;
        };

        struct RootEntry
        {
            Ptr<RHI::IGpuResource>            Resource;
            RHI::ICommonCommandList::ViewType ViewType;
        };

        struct DescriptorEntry
        {
            std::vector<Ptr<RHI::IGpuResource>> Resources;

            uint32_t Offset;

            /// <summary>
            /// Resizable array of resource descriptors.
            /// </summary>
            uint32_t Count;

            RHI::DescriptorTableParam Type      : 7;
            bool                      Instanced : 1;
        };

        struct SamplerEntry
        {
            std::vector<RHI::SamplerDesc> Descs;

            uint32_t Offset;

            /// <summary>
            /// Resizable array of sampler descriptors.
            /// </summary>
            uint32_t Count;

            bool Instanced;
        };

        using EntryVariant = std::variant<ConstantEntry, RootEntry, DescriptorEntry, SamplerEntry>;

        struct LayoutEntryMap
        {
            std::map<StringU8, EntryVariant> Entries;
            std::unique_ptr<uint8_t[]>       ConstantData;
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

        Ptr<LayoutEntryMap> m_Parameters;
    };
} // namespace Neon::Renderer