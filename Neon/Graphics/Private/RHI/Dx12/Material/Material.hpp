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
        UnqiueDescriptorHeapHandle m_LocalDescriptors;
    };
} // namespace Neon::RHI