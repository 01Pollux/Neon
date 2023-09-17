#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Shader.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    UPtr<IShader> IShader::Create(
        std::unique_ptr<uint8_t[]> Data,
        size_t                     DataSize)
    {
        return UPtr<IShader>{ NEON_NEW Dx12Shader(std::move(Data), DataSize) };
    }

    UPtr<IShader> IShader::Create(
        StringU8View             SourceCode,
        const ShaderCompileDesc& Desc,
        StringU8View             IncludeDirectory)
    {
        auto   ShaderCompiler = Dx12RenderDevice::Get()->GetShaderCompiler();
        size_t DataSize;
        auto   Data = ShaderCompiler->Compile(IncludeDirectory, SourceCode, Desc, DataSize);
        return IShader::Create(std::move(Data), DataSize);
    }

    //

    Dx12Shader::Dx12Shader(
        UPtr<uint8_t[]> ShaderData,
        size_t          DataSize) :
        m_ShaderData(std::move(ShaderData)),
        m_DataSize(DataSize)
    {
        NEON_ASSERT(m_ShaderData != nullptr, "Shader data is null.");
        NEON_ASSERT(DataSize, "Shader data size is zero.");
    }

    void Dx12Shader::CreateInputLayout(
        ShaderInputLayout& Layout)
    {
        auto ShaderCompiler = Dx12RenderDevice::Get()->GetShaderCompiler();
        ShaderCompiler->ReflectLayout(
            m_ShaderData.get(), m_DataSize, Layout, false);
    }

    void Dx12Shader::CreateOuputLayout(
        ShaderInputLayout& Layout)
    {
        auto ShaderCompiler = Dx12RenderDevice::Get()->GetShaderCompiler();
        ShaderCompiler->ReflectLayout(
            m_ShaderData.get(), m_DataSize, Layout, true);
    }

    auto Dx12Shader::GetByteCode() -> ByteCode
    {
        return { m_ShaderData.get(), m_DataSize };
    }

    Vector3U Dx12Shader::GetComputeGroupSize() const
    {
        auto ShaderCompiler = Dx12RenderDevice::Get()->GetShaderCompiler();
        auto Reflection     = ShaderCompiler->GetReflection(m_ShaderData.get(), m_DataSize);

        Vector3U Size{};
        ThrowIfFailed(Reflection->GetThreadGroupSize(&Size.x, &Size.y, &Size.z));
        return Size;
    }
} // namespace Neon::RHI