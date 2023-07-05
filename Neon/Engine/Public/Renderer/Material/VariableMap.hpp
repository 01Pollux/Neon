#pragma once

#include <Renderer/Material/Common.hpp>
#include <RHI/RootSignature.hpp>

namespace Neon::Renderer
{
    struct ShaderBinding
    {
        uint32_t Register = 0, Space = 0;
    };

    enum class MaterialVarType : uint8_t
    {
        Bool,
        Bool2,
        Bool3,
        Bool4,
        Int,
        Int2,
        Int3,
        Int4,
        UInt,
        UInt2,
        UInt3,
        UInt4,
        Float,
        Float2,
        Float3,
        Float4,
        Color,
        Matrix3x3,
        Matrix4x4,

        Buffer,
        Resource,
        RWResource,

        StaticSampler,
        DynamicSampler
    };

    class MaterialVariableMap
    {
    public:
        class View;

        /// <summary>
        /// Add a new variable to the map.
        /// </summary>
        View& Add(
            const StringU8& Name,
            ShaderBinding   Binding,
            MaterialVarType Type);

        /// <summary>
        /// Remove variable from the map.
        /// </summary>
        void Remove(
            const StringU8& Name);

        /// <summary>
        /// Add a new static sampler to the map.
        /// </summary>
        void AddSampler(
            const StringU8&         Name,
            ShaderBinding           Binding,
            RHI::ShaderVisibility   Visibility,
            const RHI::SamplerDesc& Desc);

        /// <summary>
        /// Add a common static sampler to the map.
        /// </summary>
        void AddSampler(
            const StringU8&         Name,
            ShaderBinding           Binding,
            RHI::ShaderVisibility   Visibility,
            MaterialCommon::Sampler Sampler);

        /// <summary>
        /// Remove static sampler from the map.
        /// </summary>
        void RemoveSampler(
            const StringU8& Name);

    private:
        std::list<View>                            m_Variables;
        std::map<StringU8, RHI::StaticSamplerDesc> m_StaticSamplers;
    };

    class MaterialVariableMap::View
    {
    public:
        View(StringU8        Name,
             ShaderBinding   Binding,
             MaterialVarType Type) :
            m_Name(std::move(Name)),
            m_Binding(Binding),
            m_Type(Type)
        {
        }

        /// <summary>
        /// Set name of the variable.
        /// </summary>
        View& Name(
            StringU8 Name)
        {
            m_Name = std::move(Name);
            return *this;
        }

        /// <summary>
        /// Get name of the variable.
        /// </summary>
        [[nodiscard]] const StringU8& Name() const
        {
            return m_Name;
        }

        //

        /// <summary>
        /// Set array size of the variable.
        /// </summary>
        View& ArraySize(
            size_t Count)
        {
            m_ArraySize = Count;
            return *this;
        }

        /// <summary>
        /// Get array size of the variable.
        /// </summary>
        [[nodiscard]] size_t ArraySize() const
        {
            return m_ArraySize;
        }

        //

        View& Binding(
            ShaderBinding Binding)
        {
            m_Binding = Binding;
            return *this;
        }

        /// <summary>
        /// Get binding of the variable.
        /// </summary>
        [[nodiscard]] ShaderBinding Binding() const
        {
            return m_Binding;
        }

        //

        /// <summary>
        /// Set type of the variable.
        /// </summary>
        View& Type(
            MaterialVarType VarType)
        {
            m_Type = VarType;
            return *this;
        }

        /// <summary>
        /// Get type of the variable.
        /// </summary>
        [[nodiscard]] MaterialVarType Type() const
        {
            return m_Type;
        }

    private:
        StringU8 m_Name;
        size_t   m_ArraySize = 1;

        ShaderBinding   m_Binding;
        MaterialVarType m_Type;
    };
} // namespace Neon::Renderer