#include <EnginePCH.hpp>
#include <Renderer/Material/Material.hpp>

namespace Renderer
{
    void Material::Update(
        RHI::ICommonCommandContext& Context)
    {
        m_Layout.Update(Context);
    }

    const RHI::RootSignature& Material::GetRootSignature() const
    {
        return m_Layout.GetRootSignature();
    }

    const RHI::PipelineState& Material::GetPipelineState() const
    {
        return m_PipelineState;
    }
} // namespace Renderer
