#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/OuputImage.hpp>

#include <RHI/Swapchain.hpp>

namespace Neon::RG
{
    void InitializeOutputImage::ResolveResources(
        ResourceResolver& Resolver)
    {
        auto Desc = RHI::ResourceDesc::Tex2D(
            RHI::ISwapchain::Get()->GetFormat(),
            0, 0, 1, 1);
        Desc.SetClearValue(Colors::Green);

        Resolver.CreateTexture(
            RG::ResourceId(STR("OutputImage")),
            Desc,
            MResourceFlags::FromEnum(EResourceFlags::WindowSizeDependent));
    }
} // namespace Neon::RG
