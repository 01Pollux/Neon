#include <EnginePCH.hpp>
#include <Renderer/RG/Common.hpp>

namespace Neon::RG
{
    void ResourceDesc::SetClearValue(
        RHI::EResourceFormat Format,
        const Color4&        Color)
    {
        ClearValue = ClearOperation{
            .Format = Format,
            .Color  = Color
        };
    }

    void ResourceDesc::SetClearValue(
        RHI::EResourceFormat Format,
        float                Depth,
        uint8_t              Stencil)
    {
        ClearValue = ClearOperation{
            .Format       = Format,
            .DepthStencil = { Depth, Stencil }
        };
    }

    void ResourceDesc::SetClearValue(
        const Color4& Color)
    {
        SetClearValue(ClearOperation{ .Format = Format, .Color = { Color } });
    }

    void ResourceDesc::SetClearValue(
        float   Depth,
        uint8_t Stencil)
    {
        SetClearValue(ClearOperation{ .Format = Format, .DepthStencil = { Depth, Stencil } });
    }

    void ResourceDesc::SetClearValue(
        const ClearOperationOpt& Op)
    {
        ClearValue = Op;
    }

    void ResourceDesc::UnsetClearValue()
    {
        ClearValue.reset();
    }
} // namespace Neon::RG