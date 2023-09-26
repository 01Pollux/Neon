#pragma once

#include <Asset/Handle.hpp>
#include <RHI/Material/States.hpp>

#include <RHI/Shader.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>

#include <map>

namespace Neon::RHI
{
    class IMaterial;

    template<typename _Ty, size_t _ShaderCount>
    class GenericMaterialBuilderImpl
    {
    public:
        static constexpr size_t ShaderCount = _ShaderCount;

        /// <summary>
        /// Get root signature.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IRootSignature>& RootSignature() const noexcept
        {
            return m_RootSignatuer;
        }

        /// <summary>
        /// Set root signature.
        /// </summary>
        _Ty& RootSignature(
            const Ptr<RHI::IRootSignature>& RootSig)
        {
            m_RootSignatuer = RootSig;
            return static_cast<_Ty&>(*this);
        }

        /// <summary>
        /// Get the shader modules.
        /// </summary>
        [[nodiscard]] const auto& Shaders() const
        {
            return m_ShaderModules;
        }

    protected:
        std::array<Ptr<IShader>, ShaderCount> m_ShaderModules;
        Ptr<RHI::IRootSignature>              m_RootSignatuer;
    };

    template<bool _IsCompute>
    class GenericMaterialBuilder;

    template<>
    class GenericMaterialBuilder<false> : public GenericMaterialBuilderImpl<GenericMaterialBuilder<false>, 5>
    {
    public:
        /// <summary>
        /// Build material.
        /// </summary>
        [[nodiscard]] Ptr<IMaterial> Build() const;

        /// <summary>
        /// Disable or enable vertex input.
        /// </summary>
        GenericMaterialBuilder& NoVertexInput(
            bool NoInput);

        /// <summary>
        /// Get whether vertex input is disabled.
        /// </summary>
        bool NoVertexInput() const;

    public:
        /// <summary>
        /// Set the vertex shader.
        /// </summary>
        GenericMaterialBuilder& VertexShader(
            const Ptr<IShader>& Shader);

        /// <summary>
        /// Get the vertex shader.
        /// </summary>
        [[nodiscard]] const Ptr<IShader>& VertexShader() const;

        /// <summary>
        /// Set the hull shader.
        /// </summary>
        GenericMaterialBuilder& HullShader(
            const Ptr<IShader>& Shader);

        /// <summary>
        /// Get the hull shader.
        /// </summary>
        [[nodiscard]] const Ptr<IShader>& HullShader() const;

        /// <summary>
        /// Set the domain shader.
        /// </summary>
        GenericMaterialBuilder& DomainShader(
            const Ptr<IShader>& Shader);

        /// <summary>
        /// Get the domain shader.
        /// </summary>
        [[nodiscard]] const Ptr<IShader>& DomainShader() const;

        /// <summary>
        /// Set the geometry shader.
        /// </summary>
        GenericMaterialBuilder& GeometryShader(
            const Ptr<IShader>& Shader);

        /// <summary>
        /// Get the geometry shader.
        /// </summary>
        [[nodiscard]] const Ptr<IShader>& GeometryShader() const;

        /// <summary>
        /// Set the pixel shader.
        /// </summary>
        GenericMaterialBuilder& PixelShader(
            const Ptr<IShader>& Shader);

        /// <summary>
        /// Get the pixel shader.
        /// </summary>
        [[nodiscard]] const Ptr<IShader>& PixelShader() const;

        //

    public:
        /// <summary>
        /// Set the blend state.
        /// </summary>
        GenericMaterialBuilder& Blend(
            PipelineStateBuilderG::BlendState State);

        /// <summary>
        /// Set the blend state.
        /// </summary>
        GenericMaterialBuilder& Blend(
            size_t                Index,
            MaterialStates::Blend State);

        /// <summary>
        /// Get the blend state.
        /// </summary>
        [[nodiscard]] const PipelineStateBuilderG::BlendState& Blend() const;

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
            PipelineStateBuilderG::RasterizerState State);

        /// <summary>
        /// Set the rasterizer state.
        /// </summary>
        GenericMaterialBuilder& Rasterizer(
            MaterialStates::Rasterizer State);

        /// <summary>
        /// Get the rasterizer state.
        /// </summary>
        [[nodiscard]] const PipelineStateBuilderG::RasterizerState& Rasterizer() const;

    public:
        /// <summary>
        /// Set the depth stencil state.
        /// </summary>
        GenericMaterialBuilder& DepthStencil(
            PipelineStateBuilderG::DepthStencilState State);

        /// <summary>
        /// Set the depth stencil state.
        /// </summary>
        GenericMaterialBuilder& DepthStencil(
            MaterialStates::DepthStencil State);

        /// <summary>
        /// Get the rasterizer state.
        /// </summary>
        [[nodiscard]] const PipelineStateBuilderG::DepthStencilState& DepthStencil() const;

    public:
        /// <summary>
        /// Set the input layout.
        /// </summary>
        GenericMaterialBuilder& InputLayout(
            std::optional<ShaderInputLayout> Layout);

        /// <summary>
        /// Get the input layout.
        /// </summary>
        [[nodiscard]] const std::optional<ShaderInputLayout>& InputLayout() const;

    public:
        /// <summary>
        /// Set the strip cut type.
        /// </summary>
        GenericMaterialBuilder& StripCut(
            PipelineStateBuilderG::StripCutType Type);

        /// <summary>
        /// Get the strip cut type.
        /// </summary>
        [[nodiscard]] PipelineStateBuilderG::StripCutType StripCut() const;

    public:
        /// <summary>
        /// Set the depth stencil format.
        /// </summary>
        GenericMaterialBuilder& Topology(
            PrimitiveTopologyCategory Type);

        /// <summary>
        /// Get the depth stencil format.
        /// </summary>
        [[nodiscard]] PrimitiveTopologyCategory Topology() const;

    private:
        PipelineStateBuilderG::BlendState m_BlendState;

        uint32_t m_SampleMask    = 0xFFFFFFFF;
        uint32_t m_SampleCount   = 1;
        uint32_t m_SampleQuality = 0;

        PipelineStateBuilderG::RasterizerState   m_Rasterizer;
        PipelineStateBuilderG::DepthStencilState m_DepthStencil;

        std::optional<ShaderInputLayout> m_InputLayout;

        PipelineStateBuilderG::StripCutType m_StripCut           = PipelineStateBuilderG::StripCutType::None;
        PrimitiveTopologyCategory           m_Topology           = PrimitiveTopologyCategory::Undefined;
        EResourceFormat                     m_DepthStencilFormat = EResourceFormat::Unknown;

        bool m_NoVertexInput = false;
    };

    template<>
    class GenericMaterialBuilder<true> : public GenericMaterialBuilderImpl<GenericMaterialBuilder<true>, 1>
    {
    public:
        /// <summary>
        /// Build material.
        /// </summary>
        [[nodiscard]] Ptr<IMaterial> Build() const;

        /// <summary>
        /// Set the compute shader.
        /// </summary>
        GenericMaterialBuilder& ComputeShader(
            const Ptr<IShader>& Shader);

        /// <summary>
        /// Get the compute shader.
        /// </summary>
        const Ptr<IShader>& ComputeShader() const;
    };

    using RenderMaterialBuilder  = GenericMaterialBuilder<false>;
    using ComputeMaterialBuilder = GenericMaterialBuilder<true>;

    //

    /// <summary>
    /// Get the sampler description based on the type.
    /// </summary>
    [[nodiscard]] SamplerDesc GetSamplerDesc(
        MaterialStates::Sampler Type);

    /// <summary>
    /// Get the static sampler description based on the type.
    /// </summary>
    [[nodiscard]] StaticSamplerDesc GetStaticSamplerDesc(
        MaterialStates::Sampler Type,
        uint16_t                Register,
        uint16_t                Space,
        ShaderVisibility        Visibility);
} // namespace Neon::RHI