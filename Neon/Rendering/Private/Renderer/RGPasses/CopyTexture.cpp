#include <EnginePCH.hpp>
#include <Renderer/RGPasses/CopyTexture.hpp>

#include <DirectXColors.h>

namespace Renderer::RG
{
    CopyToTexturePass::CopyToTexturePass() :
        BaseClass(PassQueueType::Direct)
    {
    }

    void CopyToTexturePass::SetupResources(
        RenderGraphPassResBuilder& PassBuilder)
    {
        PassBuilder.ReadResource(
            m_SrcTexture,
            ResourceReadAccess::PixelShader,
            ShaderResourceViewDesc{});

        PassBuilder.WriteResource(
            m_DstTexture,
            RenderTargetViewDesc{
                .ClearType = ClearType::RTV_Color });
    }

    void CopyToTexturePass::SetupRootSignatures(
        RenderGraphPassRSBuilder& PassBuilder)
    {
        PassBuilder.CreateRootSignature(
            RG_RESOURCEID("CopyToTextureRS"),
            RHI::RootSignatureDesc()
                .AddDescriptorTable(
                    RHI::RootDescriptorTable(1)
                        .AddSrvRange(0, 0, 1),
                    D3D12_SHADER_VISIBILITY_PIXEL)
                .AddSampler(
                    CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR)));
    }

    void CopyToTexturePass::SetupShaders(
        RenderGraphPassShaderBuilder& PassBuilder)
    {
        RHI::IGraphicsShader::CompileDesc Desc;

        Desc.EntryName = STR("VS_Main");
        Desc.Stage     = RHI::GraphicsShaderStage::Vertex;
        PassBuilder.LoadShader(
            RG_RESOURCEID("VS_CopyToTexture"), "Engine/Shaders/Others/CopyToTextureTmp.hlsl", Desc);

        Desc.EntryName = STR("PS_Main");
        Desc.Stage     = RHI::GraphicsShaderStage::Pixel;
        PassBuilder.LoadShader(
            RG_RESOURCEID("PS_CopyToTexture"), "Engine/Shaders/Others/CopyToTextureTmp.hlsl", Desc);
    }

    void CopyToTexturePass::SetupPipelineStates(
        RenderGraphPassPSOBuilder& PassBuilder)
    {
        auto RootSig  = PassBuilder.GetRootSignature(RG_RESOURCEID("CopyToTextureRS"));
        auto VsShader = PassBuilder.GetShader(RG_RESOURCEID("VS_CopyToTexture"));
        auto PsShader = PassBuilder.GetShader(RG_RESOURCEID("PS_CopyToTexture"));

        RHI::GraphicsPipelineStateDesc PsoDesc;
        PsoDesc.RootSignature(RootSig)
            .Topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
            .VertexShader(VsShader->GetData())
            .PixelShader(PsShader->GetData())
            .RenderTargetCount(1)
            .RenderTargetFormat(0, m_DstFormat)
            .DepthStencil(D3D12_DEPTH_STENCIL_DESC{});

        //

        PassBuilder.CreatePipelineState(
            RG_RESOURCEID("CopyToTexturePSO_Default"),
            PsoDesc);

        //

        CD3DX12_BLEND_DESC BlendState(D3D12_DEFAULT);

        auto BlendStateRtv        = BlendState.RenderTarget[0];
        BlendStateRtv.BlendEnable = true;
        BlendStateRtv.SrcBlend    = D3D12_BLEND_SRC_ALPHA;
        BlendStateRtv.DestBlend   = D3D12_BLEND_INV_SRC_ALPHA;
        BlendStateRtv.BlendOp     = D3D12_BLEND_OP_ADD;

        PsoDesc.BlendState(BlendState);

        PassBuilder.CreatePipelineState(
            RG_RESOURCEID("CopyToTexturePSO_Alpha"),
            PsoDesc);

        //

        BlendStateRtv.BlendEnable = true;
        BlendStateRtv.SrcBlend    = D3D12_BLEND_ONE;
        BlendStateRtv.DestBlend   = D3D12_BLEND_ONE;
        BlendStateRtv.BlendOp     = D3D12_BLEND_OP_ADD;

        PsoDesc.BlendState(BlendState);

        PassBuilder.CreatePipelineState(
            RG_RESOURCEID("CopyToTexturePSO_Additive"),
            PsoDesc);
    }

    //

    void CopyToTexturePass::Execute(
        const RenderGraphStorage& GraphStorage,
        RHI::ICommandContext&     CmdContext)
    {
        ResourceId PsoResId = RG_RESOURCEID("CopyToTexturePSO_Default");
        switch (m_CopyType)
        {
        case CopyType::AlphaBlend:
            PsoResId = RG_RESOURCEID("CopyToTexturePSO_Alpha");
            break;
        case CopyType::AdditiveBlend:
            PsoResId = RG_RESOURCEID("CopyToTexturePSO_Additive");
            break;
        }

        auto& GraphicsContext = static_cast<RHI::GraphicsCommandContext&>(CmdContext);

        auto& RootSig       = GraphStorage.GetRootSignature(RG_RESOURCEID("CopyToTextureRS"));
        auto& PipelineState = GraphStorage.GetPipelineState(PsoResId);

        auto& SrcTexture = GraphStorage.GetResource(m_SrcTexture.GetResource()).Get();

        GraphicsContext.SetRootSignature(RootSig);
        GraphicsContext.SetPipelineState(PipelineState);
        GraphicsContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        const auto& [Heap, Offset, Size] = GraphicsContext.GetResourceHeap();

        Heap->CreateShaderResourceView(
            Offset,
            SrcTexture->Get());

        GraphicsContext.SetDescriptorTable(0, Heap->GetGPUAddress(Offset));

        GraphicsContext.Draw(RHI::DrawArgs{
            .VertexCountPerInstance = 4 });
    }

    //

    auto CopyToTexturePass::SetSourceTexture(
        const ResourceViewId& SourceTexture) -> ThisClass&
    {
        m_SrcTexture = SourceTexture;
        return *this;
    }

    auto CopyToTexturePass::SetDestTexture(
        DXGI_FORMAT           Format,
        const ResourceViewId& DestTexture) -> ThisClass&
    {
        m_DstTexture = DestTexture;
        m_DstFormat  = Format;
        return *this;
    }

    auto CopyToTexturePass::SetCopyType(
        CopyType Type) -> ThisClass&
    {
        m_CopyType = Type;
        return *this;
    }
} // namespace Renderer::RG
