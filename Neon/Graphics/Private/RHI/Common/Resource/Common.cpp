#include <GraphicsPCH.hpp>
#include <RHI/Resource/Common.hpp>

namespace Neon::RHI
{
    void ResourceDesc::SetClearValue(
        RHI::EResourceFormat ResFormat,
        const Color4&        Color)
    {
        ClearValue = ClearOperation{
            .Format = ResFormat,
            .Value  = { Color }
        };
    }

    void ResourceDesc::SetClearValue(
        RHI::EResourceFormat ResFormat,
        float                DepthValue,
        uint8_t              StencilValue)
    {
        ClearValue = ClearOperation{
            .Format = ResFormat,
            .Value  = ClearOperation::DepthStencil{ DepthValue, StencilValue }
        };
    }

    void ResourceDesc::SetClearValue(
        const Color4& Color)
    {
        SetClearValue(Format, Color);
    }

    void ResourceDesc::SetClearValue(
        float   DepthValue,
        uint8_t StencilValue)
    {
        SetClearValue(Format, DepthValue, StencilValue);
    }

    void ResourceDesc::UnsetClearValue()
    {
        ClearValue.reset();
    }
} // namespace Neon::RHI