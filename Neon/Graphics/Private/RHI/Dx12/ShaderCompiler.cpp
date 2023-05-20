#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/ShaderCompiler.hpp>

namespace Neon::RHI
{
    Dx12ShaderCompiler::Dx12ShaderCompiler()
    {
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&m_Validator)));
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_Utils)));
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_Compiler)));
        ThrowIfFailed(m_Utils->CreateDefaultIncludeHandler(&m_DefaultIncludeHandler));
    }

    IShader::ByteCode Dx12ShaderCompiler::Compile(
        const ShaderCompileDesc& Desc)
    {
        return IShader::ByteCode();
    }

    void Dx12ShaderCompiler::ReflectInputLayout(
        const void*      ShaderCode,
        size_t           ByteLength,
        InputLayoutDesc* GraphicsLayout,
        RawBufferLayout* Layout,
        bool             IsOutput)
    {
    }

    void Dx12ShaderCompiler::ReflectInputLayout(
        ID3D12ShaderReflection* ShaderReflection,
        InputLayoutDesc*        GraphicsLayout,
        RawBufferLayout*        Layout,
        bool                    IsOutput)
    {
    }
    String Dx12ShaderCompiler::GetShaderModel(ShaderStage Stage, ShaderProfile Profile)
    {
        return String();
    }
} // namespace Neon::RHI