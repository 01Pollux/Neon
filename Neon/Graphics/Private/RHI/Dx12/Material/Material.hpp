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

    class Material final : public IMaterial
    {
        friend class MaterialInstance;
        using DescriptorHeapHandle = DescriptorHeapHandle;

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
        void SetResource(
            const StringU8&                Name,
            const Ptr<RHI::IGpuResource>&  Resource,
            const RHI::DescriptorViewDesc& Desc) override;

        void SetSampler(
            const StringU8&         Name,
            const RHI::SamplerDesc& Desc) override;

        void SetData(
            const StringU8& Name,
            const void*     Data) override;

    private:
        struct RootEntry
        {
            Structured::Layout       Struct;
            uint32_t                 Offset;
            RHI::CstResourceViewType ViewType;
        };

        struct SamplerEntry
        {
            RHI::SamplerDesc Desc;
            uint32_t         Offset;
        };

        struct DescriptorEntry
        {
            DescriptorViewDesc        Desc;
            uint32_t                  Offset;
            RHI::DescriptorTableParam Type;
        };

        using EntryVariant = std::variant<RootEntry, DescriptorEntry>;

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
            UnqiueDescriptorHeapHandle          Descriptors;
            std::map<StringU8, DescriptorEntry> Entries;
            UBufferPoolHandle                   Buffer;

            Blackboard() = default;
            Blackboard(const Blackboard& Other);
            Blackboard& operator=(const Blackboard&) = delete;
            NEON_CLASS_MOVE(Blackboard);
            ~Blackboard() = default;
        };

    private:
        Ptr<Blackboard> m_SharedParameters = std::make_shared<Blackboard>();
        Blackboard      m_LocalParameters;

        bool m_IsCompute     : 1 = false;
        bool m_IsTransparent : 1 = false;
    };
} // namespace Neon::RHI