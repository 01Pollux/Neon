#pragma once

#include <RHI/Shader.hpp>
#include <DX/d3d12shader.h>
#include <DxC/dxcapi.h>

#include <Window/Win32/WinPtr.hpp>

namespace Neon::RHI
{
    class GraphicsShader;

    class Dx12ShaderCompiler
    {
    public:
        Dx12ShaderCompiler();

        /// <summary>
        /// Compile shader from source code
        /// </summary>
        IShader::ByteCode Compile(
            const ShaderCompileDesc& Desc);

        /// <summary>
        /// Reflect shader layout from shader bytecode
        /// </summary>
        void ReflectLayout(
            const void*         ShaderCode,
            size_t              ByteLength,
            MBuffer::RawLayout& Layout,
            bool                IsOutput);

    private:
        /// <summary>
        /// Reflect shader layout from shader bytecode
        /// </summary>
        void ReflectLayout(
            ID3D12ShaderReflection* ShaderReflection,
            MBuffer::RawLayout&     Layout,
            bool                    IsOutput);

    private:
        /// <summary>
        /// Get shader model from shader stage and profile
        /// </summary>
        [[nodiscard]] static String GetShaderModel(
            ShaderStage   Stage,
            ShaderProfile Profile);

    private:
        Win32::ComPtr<IDxcValidator>      m_Validator;
        Win32::ComPtr<IDxcUtils>          m_Utils;
        Win32::ComPtr<IDxcCompiler3>      m_Compiler;
        Win32::ComPtr<IDxcIncludeHandler> m_DefaultIncludeHandler;
    };
} // namespace Neon::RHI
