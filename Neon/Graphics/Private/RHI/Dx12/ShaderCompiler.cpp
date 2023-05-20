#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/ShaderCompiler.hpp>

#include <execution>

#include <Log/Logger.hpp>

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
        Win32::ComPtr<IDxcBlobEncoding> ShaderCodeBlob;
        ThrowIfFailed(m_Utils->CreateBlobFromPinned(
            Desc.SourceCode.c_str(),
            uint32_t(Desc.SourceCode.size()),
            0,
            &ShaderCodeBlob));

        std::vector<const wchar_t*> Options;
#if !NEON_DIST
        if (Desc.Flags.Test(EShaderCompileFlags::Debug))
        {
            Options.emplace_back(DXC_ARG_DEBUG);
            Options.emplace_back(STR("-Qembed_debug"));
        }
        else
#endif
        {
            Options.emplace_back(STR("-Qstrip_debug"));
        }

#if !NEON_DIST
        if (Desc.Flags.Test(EShaderCompileFlags::SkipOptimization))
        {
            Options.emplace_back(DXC_ARG_SKIP_OPTIMIZATIONS);
        }
        else
#endif
        {
            Options.emplace_back(DXC_ARG_OPTIMIZATION_LEVEL3);
        }

        Options.emplace_back(DXC_ARG_ALL_RESOURCES_BOUND);
        Options.emplace_back(STR("-HV 2021"));

        Options.emplace_back(STR("-E"));
        Options.emplace_back(Desc.EntryPoint.c_str());

        Options.emplace_back(STR("-T"));
        String Model = GetShaderModel(Desc.Stage, Desc.Profile);
        Options.emplace_back(Model.c_str());

        std::vector<String> MacrosCombined;
        MacrosCombined.reserve(Desc.Defines.size());
        Options.reserve(Options.size() + Desc.Defines.size());

        for (auto& Macro : Desc.Defines)
        {
            MacrosCombined.emplace_back(StringUtils::Format(
                STR("{}{}"),
                Macro.first,
                Macro.second.empty() ? STR("=1") : Macro.second));
            Options.emplace_back(STR("-D"));
            Options.emplace_back(MacrosCombined.back().c_str());
        }

        DxcBuffer Buffer{
            .Ptr      = ShaderCodeBlob->GetBufferPointer(),
            .Size     = ShaderCodeBlob->GetBufferSize(),
            .Encoding = 0
        };

        Win32::ComPtr<IDxcResult> Result;
        ThrowIfFailed(m_Compiler->Compile(
            &Buffer,
            Options.data(),
            uint32_t(Options.size()),
            nullptr,
            IID_PPV_ARGS(&Result)));

        Win32::ComPtr<IDxcBlobUtf8> Error;
        if (SUCCEEDED(Result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&Error), nullptr)))
        {
            size_t StrLen = Error->GetStringLength();
            if (Error && Error->GetStringLength() > 0)
            {
                NEON_ERROR_TAG("ShaderCompiler", StringU8(Error->GetStringPointer(), StrLen));
                return {};
            }
        }

        Win32::ComPtr<IDxcBlob> Data;
        ThrowIfFailed(Result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&Data), nullptr));

        uint8_t* CompiledShaderCode = static_cast<uint8_t*>(Data->GetBufferPointer());

        size_t CodeSize = Data->GetBufferSize();
        auto   CodePtr  = std::make_unique<uint8_t[]>(CodeSize);

        std::copy(CompiledShaderCode, CompiledShaderCode + CodeSize, CodePtr.get());

        // Add validation
        {
            Win32::ComPtr<IDxcOperationResult> OperationResult;
            ThrowIfFailed(m_Validator->Validate(
                Data.Get(),
                DxcValidatorFlags_InPlaceEdit,
                &OperationResult));

            HRESULT Status;
            ThrowIfFailed(OperationResult->GetStatus(&Status));

            if (FAILED(Status))
            {
                Win32::ComPtr<IDxcBlobEncoding> Error;
                Win32::ComPtr<IDxcBlobUtf8>     ErrorUtf8;
                OperationResult->GetErrorBuffer(&Error);
                m_Utils->GetBlobAsUtf8(Error.Get(), &ErrorUtf8);

                NEON_ERROR_TAG("ShaderCompiler", StringU8(ErrorUtf8->GetStringPointer(), ErrorUtf8->GetBufferSize()));
                return {};
            }
        }

        return { CodePtr.release(), CodeSize };
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