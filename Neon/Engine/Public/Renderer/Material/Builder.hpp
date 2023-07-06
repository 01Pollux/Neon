#pragma once

#include <Renderer/Material/VariableMap.hpp>
#include <Resource/Types/Shader.hpp>
#include <RHI/PipelineState.hpp>

namespace Neon::Renderer
{
    template<typename _Ty, size_t _ShaderCount>
    class GenericMaterialBuilderImpl
    {
    public:
        static constexpr size_t ShaderCount = _ShaderCount;

        struct ShaderModuleHandle
        {
            Asset::ShaderModuleId    ModuleId;
            RHI::ShaderMacros        Macros;
            RHI::ShaderProfile       Profile = RHI::ShaderProfile::SP_6_5;
            RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default;
            bool                     Enabled = false;
        };

    public:
        /// <summary>
        /// Get the variable map.
        /// </summary>
        [[nodiscard]] MaterialVariableMap& VarMap()
        {
            return m_VarMap;
        }

        /// <summary>
        /// Get the variable map.
        /// </summary>
        [[nodiscard]] const MaterialVariableMap& VarMap() const
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
            return static_cast<_Ty&>(*this);
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
        [[nodiscard]] const ShaderModuleHandle& VertexShader() const;

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
        [[nodiscard]] const ShaderModuleHandle& HullShader() const;

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
        [[nodiscard]] const ShaderModuleHandle& DomainShader() const;

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
        [[nodiscard]] const ShaderModuleHandle& GeometryShader() const;

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
        [[nodiscard]] const ShaderModuleHandle& PixelShader() const;

        //

    public:
        /// <summary>
        /// Set the blend state.
        /// </summary>
        GenericMaterialBuilder& BlendState(
            RHI::PipelineStateBuilderG::BlendState State);

        /// <summary>
        /// Get the blend state.
        /// </summary>
        [[nodiscard]] const RHI::PipelineStateBuilderG::BlendState& BlendState() const;

    public:
        /// <summary>
        /// Set sample mask, count and quality.
        /// </summary>
        GenericMaterialBuilder& Sample(
            uint32_t Mask,
            uint32_t Count,
            uint32_t Quality);

        /// <summary>
        /// Get sample mask.
        /// </summary>
        [[nodiscard]] uint32_t SampleMask() const;

        /// <summary>
        /// Get sample count.
        /// </summary>
        [[nodiscard]] uint32_t SampleCount() const;

        /// <summary>
        /// Get sample quality.
        /// </summary>
        [[nodiscard]] uint32_t SampleQuality() const;

    public:
        /// <summary>
        /// Set the rasterizer state.
        /// </summary>
        GenericMaterialBuilder& Rasterizer(
            RHI::PipelineStateBuilderG::RasterizerState State);

        /// <summary>
        /// Get the rasterizer state.
        /// </summary>
        [[nodiscard]] const RHI::PipelineStateBuilderG::RasterizerState& Rasterizer() const;

    public:
        /// <summary>
        /// Set the depth stencil state.
        /// </summary>
        GenericMaterialBuilder& DepthStencil(
            RHI::PipelineStateBuilderG::DepthStencilState State);

        /// <summary>
        /// Get the rasterizer state.
        /// </summary>
        [[nodiscard]] const RHI::PipelineStateBuilderG::DepthStencilState& DepthStencil() const;

    public:
        /// <summary>
        /// Set the input layout.
        /// </summary>
        GenericMaterialBuilder& InputLayout(
            std::optional<RHI::ShaderInputLayout> Layout);

        /// <summary>
        /// Get the input layout.
        /// </summary>
        [[nodiscard]] const std::optional<RHI::ShaderInputLayout>& InputLayout() const;

    public:
        /// <summary>
        /// Set the strip cut type.
        /// </summary>
        GenericMaterialBuilder& StripCut(
            RHI::PipelineStateBuilderG::StripCutType Type);

        /// <summary>
        /// Get the strip cut type.
        /// </summary>
        [[nodiscard]] RHI::PipelineStateBuilderG::StripCutType StripCut() const;

    public:
        /// <summary>
        /// Set the render target format.
        /// </summary>
        GenericMaterialBuilder& RenderTarget(
            uint32_t             Index,
            const StringU8&      Name,
            RHI::EResourceFormat Format);

        /// <summary>
        /// Get the render target formats.
        /// </summary>
        [[nodiscard]] auto& RenderTargets() const
        {
            return m_RenderTargetFormats;
        }

    public:
        /// <summary>
        /// Set the depth stencil format.
        /// </summary>
        GenericMaterialBuilder& DepthStencilFormat(
            RHI::EResourceFormat Format);

        /// <summary>
        /// Get the depth stencil format.
        /// </summary>
        [[nodiscard]] RHI::EResourceFormat DepthStencilFormat() const;

    private:
        RHI::PipelineStateBuilderG::BlendState m_BlendState;

        uint32_t m_SampleMask    = 0xFFFFFFFF;
        uint32_t m_SampleCount   = 1;
        uint32_t m_SampleQuality = 0;

        RHI::PipelineStateBuilderG::RasterizerState   m_Rasterizer;
        RHI::PipelineStateBuilderG::DepthStencilState m_DepthStencil;

        std::optional<RHI::ShaderInputLayout> m_InputLayout;

        std::map<uint32_t, std::pair<StringU8, RHI::EResourceFormat>> m_RenderTargetFormats;

        RHI::PipelineStateBuilderG::StripCutType m_StripCut           = RHI::PipelineStateBuilderG::StripCutType::None;
        RHI::PrimitiveTopology                   m_Topology           = RHI::PrimitiveTopology::Undefined;
        RHI::EResourceFormat                     m_DepthStencilFormat = RHI::EResourceFormat::Unknown;
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