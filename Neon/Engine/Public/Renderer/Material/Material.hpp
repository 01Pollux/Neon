#pragma once

#include <Renderer/Material/Builder.hpp>

namespace Neon::RHI
{
    class IPipelineState;
    class ICommonCommandList;
    class IGpuResource;
} // namespace Neon::RHI

namespace Neon::Renderer
{
    class Material
    {
    public:
        /// <summary>
        /// Update the material to the GPU.
        /// </summary>
        void Update(
            RHI::ICommonCommandList* CommandList);

        /// <summary>
        /// Get the root signature of the material.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IRootSignature>& GetRootSignature() const;

        /// <summary>
        /// Get the pipeline state of the material.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IPipelineState>& GetPipelineState() const;

    public:
        /// <summary>
        /// Set a constant value to the material.
        /// </summary>
        template<typename _Ty>
            requires std::is_standard_layout_v<_Ty>
        void Set(
            const StringU8& Name,
            const _Ty&      Value,
            size_t          ArrayIndex = 0,
            size_t          Offset     = 0)
        {
            if constexpr (sizeof(_Ty) < sizeof(int))
            {
                int32_t TmpVal = Value;
                Set(Name, ArrayIndex, Offset, TmpVal);
            }
            else
            {
                m_Layout.SetConstants(Name, ArrayIndex, Offset, std::addressof(Value), sizeof(_Ty));
            }
        }

        /// <summary>
        /// Set a resource to the material.
        /// </summary>
        void Set(
            const StringU8&                Name,
            const Ptr<RHI::IGpuResource>&  Resource,
            const RHI::DescriptorViewDesc& Desc,
            size_t                         ArrayIndex = 0)
        {
            m_Layout.SetResource(Name, ArrayIndex, Resource, Desc);
        }

        /// <summary>
        /// Set a resource to the material.
        /// </summary>
        void Set(
            const StringU8&               Name,
            const Ptr<RHI::IGpuResource>& Resource,
            size_t                        ArrayIndex = 0)
        {
            m_Layout.SetResource(Name, ArrayIndex, Resource, std::nullopt);
        }

    public:
        /// <summary>
        /// Set a constant value to the material.
        /// </summary>
        template<typename _Ty>
            requires std::is_standard_layout_v<_Ty>
        [[nodiscard]] _Ty Get(
            const StringU8& Name,
            size_t          ArrayIndex = 0,
            size_t          Offset     = 0) const
        {
            if constexpr (sizeof(_Ty) < sizeof(int))
            {
                return _Ty(Get<int32_t>(Name, ArrayIndex, Offset));
            }
            else
            {
                _Ty Tmp;
                m_Layout.GetConstants(Name, ArrayIndex, Offset, &Tmp, sizeof(Tmp));
                return Tmp;
            }
        }

        /// <summary>
        /// Get a resource from the material.
        /// </summary>
        void Get(
            const StringU8&          Name,
            Ptr<RHI::IGpuResource>*  Resource,
            RHI::DescriptorViewDesc& Desc,
            size_t                   ArrayIndex = 0) const
        {
            m_Layout.GetResource(Name, ArrayIndex, Resource, &Desc);
        }

        /// <summary>
        /// Get a resource from the material.
        /// </summary>
        void Get(
            const StringU8&         Name,
            Ptr<RHI::IGpuResource>* Resource,
            size_t                  ArrayIndex = 0) const
        {
            m_Layout.GetResource(Name, ArrayIndex, Resource, nullptr);
        }

    protected:
        MaterialMetaData         m_Layout;
        Ptr<RHI::IPipelineState> m_PipelineState;
    };
} // namespace Neon::Renderer
