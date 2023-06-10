#include <GraphicsPCH.hpp>
#include <RHI/Resource/Common.hpp>

namespace Neon::RHI
{
    void ResourceDesc::SetClearValue(
        RHI::EResourceFormat Format,
        const Color4&        Color)
    {
        ClearValue = ClearOperation{
            .Format = Format,
            .Value  = { Color }
        };
    }

    void ResourceDesc::SetClearValue(
        RHI::EResourceFormat Format,
        float                Depth,
        uint8_t              Stencil)
    {
        ClearValue = ClearOperation{
            .Format = Format,
            .Value  = ClearOperation::DepthStencil{ Depth, Stencil }
        };
    }

    void ResourceDesc::SetClearValue(
        const Color4& Color)
    {
        SetClearValue(Format, Color);
    }

    void ResourceDesc::SetClearValue(
        float   Depth,
        uint8_t Stencil)
    {
        SetClearValue(Format, Depth, Stencil);
    }

    void ResourceDesc::UnsetClearValue()
    {
        ClearValue.reset();
    }
} // namespace Neon::RHI