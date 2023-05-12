#include <EnginePCH.hpp>
#include <Renderer/RGPasses/GBuffer.hpp>

namespace Renderer::RG
{
    GBufferPass::GBufferPass() :
        BaseClass(PassQueueType::Direct)
    {
    }

    void GBufferPass::SetupResources(
        RenderGraphPassResBuilder& PassBuilder)
    {
        ResourceDesc Desc = ResourceDesc::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            0, 0);
        Desc.SetClearValue(Colors::None);

        PassBuilder.CreateTexture(RG_RESOURCEID("GBufferAlbedo"), Desc, ResourceFlags::WindowSizedTexture);
        PassBuilder.CreateTexture(RG_RESOURCEID("GBufferNormal"), Desc, ResourceFlags::WindowSizedTexture);
        PassBuilder.CreateTexture(RG_RESOURCEID("GBufferEmissive"), Desc, ResourceFlags::WindowSizedTexture);

        //

        Desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        Desc.SetClearValue(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.f, 0);

        PassBuilder.CreateTexture(RG_RESOURCEID("GBufferDepth"), Desc, ResourceFlags::WindowSizedTexture);

        //

        {
            RenderTargetViewDesc RtvDesc;
            RtvDesc.ClearType = ClearType::RTV_Color;
            PassBuilder.WriteResource(RG_RESOURCEVIEW("GBufferAlbedo", "MainView"), RtvDesc);
            PassBuilder.WriteResource(RG_RESOURCEVIEW("GBufferNormal", "MainView"), RtvDesc);
            PassBuilder.WriteResource(RG_RESOURCEVIEW("GBufferEmissive", "MainView"), RtvDesc);
        }

        //

        {
            DepthStencilViewDesc DsvDesc{
                .HeapDesc = D3D12_DEPTH_STENCIL_VIEW_DESC{
                    .Format        = DXGI_FORMAT_D24_UNORM_S8_UINT,
                    .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
                    .Flags         = D3D12_DSV_FLAG_READ_ONLY_DEPTH,
                    .Texture2D     = { .MipSlice = 0 } },
                .ClearType = ClearType::DSV_Depth
            };
            PassBuilder.WriteResource(RG_RESOURCEVIEW("GBufferDepth", "MainView"), DsvDesc);
        }
    }
} // namespace Renderer::RG
