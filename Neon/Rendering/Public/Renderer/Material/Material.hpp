#pragma once

#include <Core/NeonCore.hpp>
#include <Math/Matrix.hpp>

#include <RHI/PipelineState.hpp>
#include <Renderer/Material/Metadata.hpp>

namespace Renderer
{
    class Material
    {
    public:
        /// <summary>
        /// Update the material to the GPU.
        /// </summary>
        void Update(
            RHI::ICommonCommandContext& Context);

        /// <summary>
        /// Get the root signature of the material.
        /// </summary>
        [[nodiscard]] const RHI::RootSignature& GetRootSignature() const;

        /// <summary>
        /// Get the pipeline state of the material.
        /// </summary>
        [[nodiscard]] const RHI::PipelineState& GetPipelineState() const;

    public:
        /// <summary>
        /// Set a constant value to the material.
        /// </summary>
        template<typename _Ty>
            requires std::is_fundamental_v<_Ty>
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
        /// Set a constant value to the material.
        /// </summary>
        template<typename _Ty, size_t _Size>
        void Set(
            const StringU8&                  Name,
            const Neon::MVector<_Ty, _Size>& Value,
            size_t                           ArrayIndex = 0,
            size_t                           Offset     = 0)
        {
            m_Layout.SetConstants(Name, ArrayIndex, Offset, std::addressof(Value), sizeof(Value));
        }

        /// <summary>
        /// Set a constant value to the material.
        /// </summary>
        void Set(
            const StringU8&  Name,
            const Matrix4x4& Value,
            size_t           ArrayIndex = 0,
            size_t           Offset     = 0)
        {
            m_Layout.SetConstants(Name, ArrayIndex, Offset, std::addressof(Value), sizeof(Value));
        }

        /// <summary>
        /// Set a constant value to the material.
        /// </summary>
        void Set(
            const StringU8&  Name,
            const Matrix3x3& Value,
            size_t           ArrayIndex = 0,
            size_t           Offset     = 0)
        {
            m_Layout.SetConstants(Name, ArrayIndex, Offset, std::addressof(Value), sizeof(Value));
        }

        /// <summary>
        /// Set a resource to the material.
        /// </summary>
        void Set(
            const StringU8&                   Name,
            const RHI::GraphicsResource::Ptr& Resource,
            const RHI::DescriptorViewDesc&    Desc,
            size_t                            ArrayIndex = 0)
        {
            m_Layout.SetResource(Name, ArrayIndex, Resource, Desc);
        }

        /// <summary>
        /// Set a resource to the material.
        /// </summary>
        void Set(
            const StringU8&                   Name,
            const RHI::GraphicsResource::Ptr& Resource,
            size_t                            ArrayIndex = 0)
        {
            m_Layout.SetResource(Name, ArrayIndex, Resource, std::nullopt);
        }

    public:
        /// <summary>
        /// Set a constant value to the material.
        /// </summary>
        template<typename _Ty>
            requires std::is_fundamental_v<_Ty>
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
        /// Set a constant value to the material.
        /// </summary>
        template<typename _Ty, size_t _Size>
        [[nodiscard]] Neon::MVector<_Ty, _Size> GetVector(
            const StringU8& Name,
            size_t          ArrayIndex = 0,
            size_t          Offset     = 0) const
        {
            Neon::MVector<_Ty, _Size> Tmp;
            m_Layout.GetConstants(Name, ArrayIndex, Offset, std::addressof(Tmp), sizeof(Tmp));
            return Tmp;
        }

        /// <summary>
        /// Set a constant value to the material.
        /// </summary>
        [[nodiscard]] Matrix4x4 GetMat4x4(
            const StringU8& Name,
            size_t          ArrayIndex = 0,
            size_t          Offset     = 0) const
        {
            Matrix4x4 Tmp;
            m_Layout.GetConstants(Name, ArrayIndex, Offset, std::addressof(Tmp), sizeof(Tmp));
            return Tmp;
        }

        /// <summary>
        /// Set a constant value to the material.
        /// </summary>
        [[nodiscard]] Matrix3x3 GetMat3x3(
            const StringU8& Name,
            size_t          ArrayIndex = 0,
            size_t          Offset     = 0) const
        {
            Matrix3x3 Tmp;
            m_Layout.GetConstants(Name, ArrayIndex, Offset, std::addressof(Tmp), sizeof(Tmp));
            return Tmp;
        }

        /// <summary>
        /// Set a resource to the material.
        /// </summary>
        void Get(
            const StringU8&             Name,
            RHI::GraphicsResource::Ptr* Resource,
            RHI::DescriptorViewDesc*    Desc,
            size_t                      ArrayIndex = 0) const
        {
            m_Layout.GetResource(Name, ArrayIndex, Resource, Desc);
        }

    protected:
        MaterialMetaData   m_Layout;
        RHI::PipelineState m_PipelineState;
    };
} // namespace Renderer
