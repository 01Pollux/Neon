#pragma once

#include <RHI/Shader.hpp>
#include <DX/d3d12shader.h>
#include <DxC/dxcapi.h>

#include <Private/Windows/API/WinPtr.hpp>

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
        std::unique_ptr<uint8_t[]> Compile(
            const ShaderCompileDesc& Desc,
            size_t&                  DataSize);

        /// <summary>
        /// Reflect shader layout from shader bytecode
        /// </summary>
        void ReflectLayout(
            const void*        ShaderCode,
            size_t             ByteLength,
            ShaderInputLayout& Layout,
            bool               IsOutput);

    private:
        /// <summary>
        /// Reflect shader layout from shader bytecode
        /// </summary>
        void ReflectLayout(
            ID3D12ShaderReflection* ShaderReflection,
            ShaderInputLayout&      Layout,
            bool                    IsOutput);

    private:
        /// <summary>
        /// Get shader model from shader stage and profile
        /// </summary>
        [[nodiscard]] static String GetShaderModel(
            ShaderStage   Stage,
            ShaderProfile Profile);

    private:
        WinAPI::ComPtr<IDxcValidator>      m_Validator;
        WinAPI::ComPtr<IDxcUtils>          m_Utils;
        WinAPI::ComPtr<IDxcCompiler3>      m_Compiler;
        WinAPI::ComPtr<IDxcIncludeHandler> m_DefaultIncludeHandler;
    };
} // namespace Neon::RHI
