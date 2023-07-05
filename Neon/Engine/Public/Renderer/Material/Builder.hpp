#pragma once

#include <Renderer/Material/VariableMap.hpp>
#include <Resource/Types/Shader.hpp>

namespace Neon::Renderer
{
    template<typename _Ty, size_t _ShaderCount>
    class GenericMaterialBuilderImpl
    {
    public:
        static constexpr size_t ShaderCount = _ShaderCount;

        struct ShaderModuleHandle
        {
            RHI::ShaderMacros        Macros;
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5;
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default;
        };

    public:
        /// <summary>
        /// Get the variable map.
        /// </summary>
        [[nodiscard]] MaterialVariableMap& VarMap()
        {
            return m_VarMap;
        }

        //

        /// <summary>
        /// Get the shader library.
        /// </summary>
        [[nodiscard]] const Ptr<Asset::ShaderLibraryAsset>& ShaderLibrary() const
        {
            return m_ShaderLibrary;
        }

        /// <summary>
        /// Get the shader module id.
        /// </summary>
        [[nodiscard]] const Ptr<Asset::ShaderLibraryAsset>& ShaderModuleId() const
        {
            return m_ShaderLibrary;
        }

        /// <summary>
        /// Set the shader library.
        /// </summary>
        _Ty& ShaderLibrary(
            const Ptr<Asset::ShaderLibraryAsset>& ShaderLibrary,
            Asset::ShaderModuleId                 ShaderModuleId)
        {
            m_ShaderModules  = {};
            m_ShaderLibrary  = ShaderLibrary;
            m_ShaderModuleId = ShaderModuleId;
            return *this;
        }

    protected:
        std::array<ShaderModuleHandle, ShaderCount> m_ShaderModules;

    private:
        MaterialVariableMap            m_VarMap;
        Ptr<Asset::ShaderLibraryAsset> m_ShaderLibrary;
        Asset::ShaderModuleId          m_ShaderModuleId;
    };

    template<bool _IsCompute>
    class GenericMaterialBuilder;

    template<>
    class GenericMaterialBuilder<false> : public GenericMaterialBuilderImpl<GenericMaterialBuilder<false>, 5>
    {
    public:
        /// <summary>
        /// Set the vertex shader.
        /// </summary>
        GenericMaterialBuilder& VertexShader(
            const RHI::ShaderMacros& Macros,
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Get the vertex shader.
        /// </summary>
        const ShaderModuleHandle& VertexShader() const;

        /// <summary>
        /// Set the hull shader.
        /// </summary>
        GenericMaterialBuilder& HullShader(
            const RHI::ShaderMacros& Macros,
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Get the hull shader.
        /// </summary>
        const ShaderModuleHandle& HullShader() const;

        /// <summary>
        /// Set the domain shader.
        /// </summary>
        GenericMaterialBuilder& DomainShader(
            const RHI::ShaderMacros& Macros,
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Get the domain shader.
        /// </summary>
        const ShaderModuleHandle& DomainShader() const;

        /// <summary>
        /// Set the geometry shader.
        /// </summary>
        GenericMaterialBuilder& GeometryShader(
            const RHI::ShaderMacros& Macros,
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Get the geometry shader.
        /// </summary>
        const ShaderModuleHandle& GeometryShader() const;

        /// <summary>
        /// Set the pixel shader.
        /// </summary>
        GenericMaterialBuilder& PixelShader(
            const RHI::ShaderMacros& Macros,
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Get the pixel shader.
        /// </summary>
        const ShaderModuleHandle& PixelShader() const;
    };

    template<>
    class GenericMaterialBuilder<true> : public GenericMaterialBuilderImpl<GenericMaterialBuilder<true>, 1>
    {
    public:
        /// <summary>
        /// Set the compute shader.
        /// </summary>
        GenericMaterialBuilder& ComputeShader(
            const RHI::ShaderMacros& Macros,
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Get the compute shader.
        /// </summary>
        const ShaderModuleHandle& ComputeShader() const;
    };

    using RenderMaterialBuilder  = GenericMaterialBuilder<false>;
    using ComputeMaterialBuilder = GenericMaterialBuilder<true>;
} // namespace Neon::Renderer