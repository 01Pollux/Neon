#include <EnginePCH.hpp>
#include <Renderer/Material/Material.hpp>

namespace Neon::Renderer
{
    void Material::Update(
        RHI::ICommonCommandList* CommandList)
    {
        m_Layout.Update(CommandList);
    }

    const Ptr<RHI::IRootSignature>& Material::GetRootSignature() const
    {
        return m_Layout.GetRootSignature();
    }

    const Ptr<RHI::IPipelineState>& Material::GetPipelineState() const
    {
        return m_PipelineState;
    }
} // namespace Neon::Renderer
