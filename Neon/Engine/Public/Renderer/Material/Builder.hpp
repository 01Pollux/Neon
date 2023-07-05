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
            Asset::ShaderModuleId    ModuleId = -1;
            RHI::ShaderMacros        Macros;
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5;
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default;
            bool                     Enabled;
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
        /// Set the shader library.
        /// </summary>
        _Ty& ShaderLibrary(
            const Ptr<Asset::ShaderLibraryAsset>& ShaderLibrary)
        {
            m_ShaderModules = {};
            m_ShaderLibrary = ShaderLibrary;
            return *this;
        }

    protected:
        std::array<ShaderModuleHandle, ShaderCount> m_ShaderModules;

    private:
        MaterialVariableMap            m_VarMap;
        Ptr<Asset::ShaderLibraryAsset> m_ShaderLibrary;
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
            Asset::ShaderModuleId    Module,
            const RHI::ShaderMacros& Macros  = {},
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Unset the vertex shader.
        /// </summary>
        GenericMaterialBuilder& RemoveVertexShader();

        /// <summary>
        /// Get the vertex shader.
        /// </summary>
        const ShaderModuleHandle& VertexShader() const;

        /// <summary>
        /// Set the hull shader.
        /// </summary>
        GenericMaterialBuilder& HullShader(
            Asset::ShaderModuleId    Module,
            const RHI::ShaderMacros& Macros  = {},
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Unset the hull shader.
        /// </summary>
        GenericMaterialBuilder& RemoveHullShader();

        /// <summary>
        /// Get the hull shader.
        /// </summary>
        const ShaderModuleHandle& HullShader() const;

        /// <summary>
        /// Set the domain shader.
        /// </summary>
        GenericMaterialBuilder& DomainShader(
            Asset::ShaderModuleId    Module,
            const RHI::ShaderMacros& Macros  = {},
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Unset the domain shader.
        /// </summary>
        GenericMaterialBuilder& RemoveDomainShader();

        /// <summary>
        /// Get the domain shader.
        /// </summary>
        const ShaderModuleHandle& DomainShader() const;

        /// <summary>
        /// Set the geometry shader.
        /// </summary>
        GenericMaterialBuilder& GeometryShader(
            Asset::ShaderModuleId    Module,
            const RHI::ShaderMacros& Macros  = {},
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Unset the geometry shader.
        /// </summary>
        GenericMaterialBuilder& RemoveGeometryShader();

        /// <summary>
        /// Get the geometry shader.
        /// </summary>
        const ShaderModuleHandle& GeometryShader() const;

        /// <summary>
        /// Set the pixel shader.
        /// </summary>
        GenericMaterialBuilder& PixelShader(
            Asset::ShaderModuleId    Module,
            const RHI::ShaderMacros& Macros  = {},
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Unset the pixel shader.
        /// </summary>
        GenericMaterialBuilder& RemovePixelShader();

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
            Asset::ShaderModuleId    Module,
            const RHI::ShaderMacros& Macros  = {},
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5,
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default);

        /// <summary>
        /// Unset the compute shader.
        /// </summary>
        GenericMaterialBuilder& RemoveComputeShader();

        /// <summary>
        /// Get the compute shader.
        /// </summary>
        const ShaderModuleHandle& ComputeShader() const;
    };

    using RenderMaterialBuilder  = GenericMaterialBuilder<false>;
    using ComputeMaterialBuilder = GenericMaterialBuilder<true>;
} // namespace Neon::Renderer