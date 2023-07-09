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

        void Bind(
            RHI::IGraphicsCommandList* CommandList) override;

        void GetDescriptor(
            bool                       Local,
            RHI::DescriptorHeapHandle* OutResourceDescriptor,
            RHI::DescriptorHeapHandle* OutSamplerDescriptor) const override;

        const Ptr<RHI::IRootSignature>& GetRootSignature() const override;

        const Ptr<RHI::IPipelineState>& GetPipelineState() const override;

    public:
        void SetResource(
            const std::string&             Name,
            const Ptr<RHI::IGpuResource>   Resource,
            const RHI::DescriptorViewDesc& Desc,
            size_t                         ArrayIndex) override;

    private:
        /// <summary>
        /// Create a default material instance.
        /// </summary>
        void CreateSharedDescriptor(
            uint32_t Shared,
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

        using LayoutEntryMap = std::map<StringU8, LayoutEntry>;

    private:
        struct UnqiueDescriptorHeapHandle
        {
            DescriptorHeapHandle ResourceDescriptors, SamplerDescriptors;

            UnqiueDescriptorHeapHandle() = default;
            UnqiueDescriptorHeapHandle(
                uint32_t ResourceDescriptorSize,
                uint32_t SamplerDescriptorSize);

            NEON_CLASS_NO_COPY(UnqiueDescriptorHeapHandle);

            UnqiueDescriptorHeapHandle(UnqiueDescriptorHeapHandle&& Other);
            UnqiueDescriptorHeapHandle& operator=(UnqiueDescriptorHeapHandle&& Other);

            ~UnqiueDescriptorHeapHandle();
        };

    private:
        Ptr<RHI::IRootSignature> m_RootSignature;
        Ptr<RHI::IPipelineState> m_PipelineState;

        Ptr<UnqiueDescriptorHeapHandle> m_SharedDescriptors;
        UnqiueDescriptorHeapHandle      m_LocalDescriptors;

        Ptr<LayoutEntryMap> m_EntryMap;
    };
} // namespace Neon::Renderer