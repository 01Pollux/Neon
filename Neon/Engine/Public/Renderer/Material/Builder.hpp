#pragma once

#include <Asset/Handle.hpp>
#include <Renderer/Material/Common.hpp>
#include <RHI/Shader.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>
#include <map>

namespace Neon::Renderer
{
    class IMaterial;

    template<typename _Ty, size_t _ShaderCount>
    class GenericMaterialBuilderImpl
    {
    public:
        static constexpr size_t ShaderCount = _ShaderCount;

    public:
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

    protected:
        std::array<Ptr<RHI::IShader>, ShaderCount> m_ShaderModules;

    private:
        Ptr<RHI::IRootSignature> m_RootSignatuer;
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
        Ptr<IMaterial> Build();

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
            const Ptr<RHI::IShader>& Shader);

        /// <summary>
        /// Get the vertex shader.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IShader>& VertexShader() const;

        /// <summary>
        /// Set the hull shader.
        /// </summary>
        GenericMaterialBuilder& HullShader(
            const Ptr<RHI::IShader>& Shader);

        /// <summary>
        /// Get the hull shader.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IShader>& HullShader() const;

        /// <summary>
        /// Set the domain shader.
        /// </summary>
        GenericMaterialBuilder& DomainShader(
            const Ptr<RHI::IShader>& Shader);

        /// <summary>
        /// Get the domain shader.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IShader>& DomainShader() const;

        /// <summary>
        /// Set the geometry shader.
        /// </summary>
        GenericMaterialBuilder& GeometryShader(
            const Ptr<RHI::IShader>& Shader);

        /// <summary>
        /// Get the geometry shader.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IShader>& GeometryShader() const;

        /// <summary>
        /// Set the pixel shader.
        /// </summary>
        GenericMaterialBuilder& PixelShader(
            const Ptr<RHI::IShader>& Shader);

        /// <summary>
        /// Get the pixel shader.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IShader>& PixelShader() const;

        //

    public:
        /// <summary>
        /// Set the blend state.
        /// </summary>
        GenericMaterialBuilder& Blend(
            RHI::PipelineStateBuilderG::BlendState State);

        /// <summary>
        /// Set the blend state.
        /// </summary>
        GenericMaterialBuilder& Blend(
            size_t                Index,
            MaterialStates::Blend State);

        /// <summary>
        /// Get the blend state.
        /// </summary>
        [[nodiscard]] const RHI::PipelineStateBuilderG::BlendState& Blend() const;

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
        /// Set the rasterizer state.
        /// </summary>
        GenericMaterialBuilder& Rasterizer(
            MaterialStates::Rasterizer State);

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
        /// Set the depth stencil state.
        /// </summary>
        GenericMaterialBuilder& DepthStencil(
            MaterialStates::DepthStencil State);

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
            RHI::EResourceFormat Format);

        /// <summary>
        /// Get the render target formats.
        /// </summary>
        [[nodiscard]] auto& RenderTargets()
        {
            return m_RenderTargetFormats;
        }

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
        GenericMaterialBuilder& Topology(
            RHI::PrimitiveTopologyCategory Type);

        /// <summary>
        /// Get the depth stencil format.
        /// </summary>
        [[nodiscard]] RHI::PrimitiveTopologyCategory Topology() const;

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

        std::map<uint32_t, RHI::EResourceFormat> m_RenderTargetFormats;

        RHI::PipelineStateBuilderG::StripCutType m_StripCut           = RHI::PipelineStateBuilderG::StripCutType::None;
        RHI::PrimitiveTopologyCategory           m_Topology           = RHI::PrimitiveTopologyCategory::Undefined;
        RHI::EResourceFormat                     m_DepthStencilFormat = RHI::EResourceFormat::Unknown;
        bool                                     m_NoVertexInput      = false;
    };

    template<>
    class GenericMaterialBuilder<true> : public GenericMaterialBuilderImpl<GenericMaterialBuilder<true>, 1>
    {
    public:
        /// <summary>
        /// Build material.
        /// </summary>
        Ptr<IMaterial> Build();

        /// <summary>
        /// Set the compute shader.
        /// </summary>
        GenericMaterialBuilder& ComputeShader(
            const Ptr<RHI::IShader>& Shader);

        /// <summary>
        /// Get the compute shader.
        /// </summary>
        const Ptr<RHI::IShader>& ComputeShader() const;
    };

    using RenderMaterialBuilder  = GenericMaterialBuilder<false>;
    using ComputeMaterialBuilder = GenericMaterialBuilder<true>;

    //

    /// <summary>
    /// Get the sampler description based on the type.
    /// </summary>
    [[nodiscard]] RHI::SamplerDesc GetSamplerDesc(
        MaterialStates::Sampler Type);

    /// <summary>
    /// Get the static sampler description based on the type.
    /// </summary>
    [[nodiscard]] RHI::StaticSamplerDesc GetStaticSamplerDesc(
        MaterialStates::Sampler Type,
        uint16_t                Register,
        uint16_t                Space,
        RHI::ShaderVisibility   Visibility);
} // namespace Neon::Renderer