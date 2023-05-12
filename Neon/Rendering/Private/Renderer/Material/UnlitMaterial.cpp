#include <EnginePCH.hpp>
#include <Renderer/Material/UnlitMaterial.hpp>

#include <RHI/Shader.hpp>
#include <Resource/ResourceManager.hpp>
#include <Resource/Handler/BinaryResourceFile.hpp>

namespace Renderer
{
    UnlitMaterial::UnlitMaterial()
    {
        CreateMetaData();
        CreatePipelineState();

        Set("MaterialColor", Vector4D(.5f, .75f, .25f, 1.0f));
        Set("MaterialColor", Vector4D(.5f, .75f, .25f, 1.0f), 3);
    }

    void UnlitMaterial::CreateMetaData()
    {
        MaterialMetaDataBuilder Builder;
        Builder
            .AppendConstant("g_MaterialColor", sizeof(Vector4D), 5)
            .SetBinding(0)
            .SetReference("MaterialColor")
            .SetShader(ShaderVisibility::All);
        Builder
            .AppendResource("g_TestBuffer", MaterialDataType::ConstantBuffer, 3)
            .SetBinding(10, 1)
            .SetReference("TestBuffer")
            .PreferDescriptor(true)
            .SetDescriptor(0, RHI::ConstantBufferViewDesc{ 0, 256 })
            .SetDescriptor(1, RHI::ConstantBufferViewDesc{ 0, 256 })
            .SetDescriptor(2, RHI::ConstantBufferViewDesc{ 0, 256 })
            .SetShader(ShaderVisibility::Pixel);
        m_Layout = MaterialMetaData(Builder);
    }

    void UnlitMaterial::CreatePipelineState()
    {
        Asset::ResourceHandle Handle("Engine/Shaders/Box.hlsl");

        auto BinaryResource = Asset::IResourceManager::Get()->GetResourceAs<Asset::BinaryResourceFile>(Handle);
        auto ResourceView   = BinaryResource->Get().view();

        RHI::IGraphicsShader::CompileDesc ShaderDesc;
        RHI::IGraphicsShader::UPtr        VsShader, PsShader;

        ShaderDesc.ShaderCode = StringU8View(std::bit_cast<const char*>(ResourceView.data()), ResourceView.size());

        ShaderDesc.EntryName = STR("VS_Main");
        ShaderDesc.Stage     = RHI::GraphicsShaderStage::Vertex;
        VsShader.reset(RHI::IGraphicsShader::Load(ShaderDesc));

        ShaderDesc.EntryName = STR("PS_Main");
        ShaderDesc.Stage     = RHI::GraphicsShaderStage::Pixel;
        PsShader.reset(RHI::IGraphicsShader::Load(ShaderDesc));

        m_PipelineState =
            RHI::GraphicsPipelineStateDesc()
                .RootSignature(GetRootSignature().Get())
                .InputLayout(VsShader.get())
                .Topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
                .VertexShader(VsShader->GetData())
                .PixelShader(PsShader->GetData())
                .RenderTargetCount(3)
                .RenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM)
                .RenderTargetFormat(1, DXGI_FORMAT_R8G8B8A8_UNORM)
                .RenderTargetFormat(2, DXGI_FORMAT_R8G8B8A8_UNORM)
                .DepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
    }
} // namespace Renderer
