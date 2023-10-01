#include <EnginePCH.hpp>
#include <RHI/Shaders/Common.hpp>
#include <Asset/Manager.hpp>

namespace Neon::RHI
{
    GlobalShader::GlobalShader(
        const Asset::Handle& Handle) :
        m_Shader(Asset::Manager::LoadAsync(Handle))
    {
    }
} // namespace Neon::RHI