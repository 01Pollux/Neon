#pragma once

#include <RHI/Material/Material.hpp>

#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Resource/Resource.hpp>

#include <RHI/Commands/List.hpp>
#include <RHI/RootSignature.hpp>

namespace Neon::RHI
{
    class MaterialInstance;

    template<bool>
    class GenericMaterialBuilder;

    class Material final : public IMaterial
    {
        friend class MaterialInstance;
        using DescriptorHeapHandle = DescriptorHeapHandle;

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

        bool IsCompute() const noexcept override;

        bool IsTransparent() const noexcept override;

        void SetTransparent(
            bool State) noexcept override;

    public:
        void BindSharedParams(
            ICommandList* CommandList) override;

        void BindLocalParams(
            ICommandList* CommandList) override;

    public:
        void SetResource(
            const StringU8&                Name,
            const Ptr<RHI::IGpuResource>&  Resource,
            const RHI::DescriptorViewDesc& Desc,
            uint32_t                       ArrayIndex,
            const Ptr<RHI::IGpuResource>&  UavCounter) override;

        void SetSampler(
            const StringU8&         Name,
            const RHI::SamplerDesc& Desc,
            uint32_t                ArrayIndex = 0) override;

        void SetConstant(
            const StringU8& Name,
            const void*     Data,
            size_t          Size,
            uint32_t        Offset) override;

        void SetResourceView(
            const StringU8&        Name,
            RHI::GpuResourceHandle Handle) override;

        void SetDynamicResourceView(
            const StringU8&          Name,
            RHI::CstResourceViewType Type,
            const void*              Data,
            size_t                   Size) override;

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
            RHI::GpuResourceHandle   Handle{};
            RHI::CstResourceViewType ViewType;
        };

        struct DescriptorEntry
        {
            std::vector<Ptr<RHI::IGpuResource>> Resources;
            std::vector<DescriptorViewDesc>     Descs;

            uint32_t Offset;

            /// <summary>
            /// Resizable array of resource descriptors.
            /// </summary>
            uint32_t Count;

            RHI::DescriptorTableParam Type;
        };

        struct SamplerEntry
        {
            std::vector<RHI::SamplerDesc> Descs;

            uint32_t Offset;

            /// <summary>
            /// Resizable array of sampler descriptors.
            /// </summary>
            uint32_t Count;
        };

        using DescriptorVariant = std::variant<DescriptorEntry, SamplerEntry>;

        struct DescriptorVariantMap
        {
            std::map<StringU8, DescriptorVariant> Entries;

            uint32_t Offset;

            [[nodiscard]] bool IsSampler() const noexcept
            {
                return Entries.begin()->second.index() == 1;
            }
        };

        using SharedEntryVariant = std::variant<ConstantEntry, RootEntry, DescriptorVariantMap>;
        using LocalEntryVariant  = DescriptorVariantMap;

        struct UnqiueDescriptorHeapHandle
        {
            DescriptorHeapHandle ResourceDescriptors, SamplerDescriptors;

            UnqiueDescriptorHeapHandle() = default;
            UnqiueDescriptorHeapHandle(
                uint32_t ResourceDescriptorSize,
                uint32_t SamplerDescriptorSize);

            UnqiueDescriptorHeapHandle(const UnqiueDescriptorHeapHandle&);
            UnqiueDescriptorHeapHandle& operator=(const UnqiueDescriptorHeapHandle&);
            NEON_CLASS_MOVE_DECL(UnqiueDescriptorHeapHandle);

            ~UnqiueDescriptorHeapHandle();

        private:
            /// <summary>
            /// Release current descriptors
            /// </summary>
            void Release();
        };

    private:
        /// <summary>
        /// Split resource name into resource group name and resource name.
        /// </summary>
        [[nodiscard]] std::pair<StringU8, StringU8> SplitResourceName(
            const StringU8& Name);

        /// <summary>
        /// Get descriptor variant from shared/local parameters.
        /// </summary>
        DescriptorVariant* GetDescriptorVariant(
            const StringU8&              Name,
            const StringU8&              EntryName,
            DescriptorVariantMap**       VariantMap     = nullptr,
            UnqiueDescriptorHeapHandle** DescriptorHeap = nullptr);

    public:
        struct SharedParameters
        {
            UnqiueDescriptorHeapHandle             Descriptors;
            std::map<StringU8, SharedEntryVariant> SharedEntries;
            std::unique_ptr<uint8_t[]>             ConstantData;
        };

        struct LocalParameters
        {
            UnqiueDescriptorHeapHandle            Descriptors;
            std::map<StringU8, LocalEntryVariant> LocalEntries;
        };

    private:
        Ptr<SharedParameters> m_SharedParameters = std::make_shared<SharedParameters>();
        LocalParameters       m_LocalParameters;

        bool m_IsCompute     : 1 = false;
        bool m_IsTransparent : 1 = false;
    };
} // namespace Neon::RHI