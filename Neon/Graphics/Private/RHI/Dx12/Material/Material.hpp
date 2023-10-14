#pragma once

#include <Utils/Struct.hpp>
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

    class MaterialVarBuilder;

    class Material final : public IMaterial
    {
    private:
        struct RootEntry
        {
            UBufferPoolHandle  Data;
            uint8_t*           MappedData;
            Structured::Layout Struct;

            RootEntry() = default;
            RootEntry(const RootEntry& Other);
            RootEntry& operator=(const RootEntry&) = delete;
            NEON_CLASS_MOVE(RootEntry);
            ~RootEntry() = default;
        };

        struct ResourceEntry
        {
            RHI::DescriptorViewDesc Desc;
            Ptr<IGpuResource>       Resource;
            uint32_t                Offset;
        };

        struct SamplerEntry
        {
            std::optional<RHI::SamplerDesc> Desc;
            uint32_t                        Offset;
        };

        using EntryVariant = std::variant<ResourceEntry, SamplerEntry>;

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

        struct Blackboard
        {
            UnqiueDescriptorHeapHandle       Descriptors;
            std::map<StringU8, EntryVariant> Entries;
            RootEntry                        Buffer;
        };

    private:
        friend class MaterialInstance;
        using DescriptorHeapHandle = DescriptorHeapHandle;

        template<bool _Compute>
        friend void Material_CreatePipelineState(
            const GenericMaterialBuilder<_Compute>& Builder,
            Material*                               Mat);

        friend void Material_CreateParameterBlock(
            const MaterialVarBuilder& VarBuilder,
            Material*                 Mat);

        friend void Material_UpdateParameterBlock(
            const Blackboard& Board);

    public:
        Material(
            const GenericMaterialBuilder<true>& Builder);

        Material(
            const GenericMaterialBuilder<false>& Builder);

        Material(
            Material* Other);

        NEON_CLASS_NO_COPYMOVE(Material);

        ~Material() override;

        Ptr<IMaterial> CreateInstance() override;

        bool IsCompute() const noexcept override;

        bool IsTransparent() const noexcept override;

        void SetTransparent(
            bool State) noexcept override;

    public:
        void ReallocateShared() override;

        void ReallocateLocal() override;

    public:
        GpuResourceHandle GetSharedBlock() override;

        GpuResourceHandle GetLocalBlock() override;

    public:
        void SetResource(
            const StringU8&                Name,
            const Ptr<IGpuResource>&       Resource,
            const RHI::DescriptorViewDesc& Desc) override;

        void SetSampler(
            const StringU8&       Name,
            const SamplerDescOpt& Desc) override;

        void SetData(
            const StringU8& Name,
            const void*     Data,
            size_t          ArrayOffset) override;

    public:
        bool GetResource(
            const StringU8&     Name,
            Ptr<IGpuResource>*  Resource,
            DescriptorViewDesc* Desc) override;

        bool GetSampler(
            const StringU8& Name,
            SamplerDescOpt* Desc) override;

        size_t GetData(
            const StringU8& Name,
            const void*     Data,
            size_t          ArrayOffset) override;

    private:
        Ptr<Blackboard> m_SharedParameters = std::make_shared<Blackboard>();
        Blackboard      m_LocalParameters;

        bool m_IsCompute     : 1 = false;
        bool m_IsTransparent : 1 = false;
    };
} // namespace Neon::RHI