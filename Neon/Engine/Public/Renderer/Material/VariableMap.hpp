#pragma once

#include <Core/BitMask.hpp>
#include <Renderer/Material/Common.hpp>
#include <RHI/RootSignature.hpp>

namespace Neon::Renderer
{
    struct ShaderBinding
    {
        uint32_t Register = 0, Space = 0;
        auto     operator<=>(const ShaderBinding& Other) const noexcept = default;
    };

    enum class EMaterialVarFlags : uint8_t
    {
        /// <summary>
        /// Variable is shared between all instances of materials.
        /// This assumes the resource won't be unified into global resource table.
        /// </summary>
        Shared,

        _Last_Enum
    };
    using MMaterialVarFlags = Bitmask<EMaterialVarFlags>;

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
        void AddStaticSampler(
            const StringU8&         Name,
            ShaderBinding           Binding,
            RHI::ShaderVisibility   Visibility,
            const RHI::SamplerDesc& Desc);

        /// <summary>
        /// Add a common static sampler to the map.
        /// </summary>
        void AddStaticSampler(
            const StringU8&         Name,
            ShaderBinding           Binding,
            RHI::ShaderVisibility   Visibility,
            MaterialStates::Sampler Sampler);

        /// <summary>
        /// Remove static sampler from the map.
        /// </summary>
        void RemoveStaticSampler(
            const StringU8& Name);

        /// <summary>
        /// Iterate over all variables.
        /// </summary>
        template<typename _Ty>
            requires std::is_invocable_v<_Ty, View&>
        void ForEachVariable(
            _Ty Callback) const
        {
            for (auto& Var : m_Variables)
            {
                Callback(Var);
            }
        }

        /// <summary>
        /// Iterate over all static samplers.
        /// </summary>
        template<typename _Ty>
            requires std::is_invocable_v<_Ty, const RHI::StaticSamplerDesc&>
        void ForEachStaticSampler(
            _Ty Callback) const
        {
            for (auto& Desc : m_StaticSamplers | std::views::values)
            {
                Callback(Desc);
            }
        }

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
            uint32_t Count)
        {
            m_ArraySize = std::max(Count, 1u);
            return *this;
        }

        /// <summary>
        /// Get array size of the variable.
        /// </summary>
        [[nodiscard]] uint32_t ArraySize() const
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

        //

        /// <summary>
        /// Set visibility of the variable.
        /// </summary>
        View& Visibility(
            RHI::ShaderVisibility Visibility)
        {
            m_Visibility = Visibility;
            return *this;
        }

        /// <summary>
        /// Get visibility of the variable.
        /// </summary>
        [[nodiscard]] RHI::ShaderVisibility Visibility() const
        {
            return m_Visibility;
        }

        //

        /// <summary>
        /// Set the material variable flags.
        /// </summary>
        View& Flags(
            const MMaterialVarFlags& Flags)
        {
            m_Flags = Flags;
            return *this;
        }

        /// <summary>
        /// Set the material variable flags.
        /// </summary>
        View& Flags(
            EMaterialVarFlags Type,
            bool              State)
        {
            m_Flags.Set(Type, State);
            return *this;
        }

        /// <summary>
        /// Get visibility of the variable.
        /// </summary>
        [[nodiscard]] const MMaterialVarFlags& Flags() const
        {
            return m_Flags;
        }

    private:
        StringU8 m_Name;
        uint32_t m_ArraySize = 1;

        ShaderBinding         m_Binding;
        MMaterialVarFlags     m_Flags;
        MaterialVarType       m_Type;
        RHI::ShaderVisibility m_Visibility = RHI::ShaderVisibility::All;
    };
} // namespace Neon::Renderer