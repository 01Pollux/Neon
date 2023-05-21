#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Shader.hpp>
#include <Private/RHI/Dx12/Device.hpp>

namespace Neon::RHI
{
    IShader* IShader::Create(
        const ByteCode& CompiledCode)
    {
        return NEON_NEW Dx12Shader(UPtr<uint8_t[]>(CompiledCode.Data), CompiledCode.Size);
    }

    IShader* IShader::Create(
        const ShaderCompileDesc& Desc)
    {
        auto ShaderCompiler = Dx12RenderDevice::Get()->GetShaderCompiler();
        return IShader::Create(ShaderCompiler->Compile(Desc));
    }

    //

    Dx12Shader::Dx12Shader(
        UPtr<uint8_t[]> ShaderData,
        size_t          DataSize) :
        m_ShaderData(std::move(ShaderData)),
        m_DataSize(DataSize)
    {
    }

    void Dx12Shader::CreateInputLayout(
        MBuffer::RawLayout& Layout)
    {
        auto ShaderCompiler = Dx12RenderDevice::Get()->GetShaderCompiler();
        ShaderCompiler->ReflectLayout(
            m_ShaderData.get(), m_DataSize, Layout, false);
    }

    void Dx12Shader::CreateOuputLayout(
        MBuffer::RawLayout& Layout)
    {
    }

    auto Dx12Shader::GetByteCode() -> ByteCode
    {
        return { m_ShaderData.get(), m_DataSize };
    }
} // namespace Neon::RHI