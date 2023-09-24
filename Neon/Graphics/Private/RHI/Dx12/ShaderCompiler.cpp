#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/ShaderCompiler.hpp>

#include <FileSystem/Path.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Types/TextFile.hpp>

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

    class ReflectionBlob : public IDxcBlob
    {
    public:
        ReflectionBlob(
            const void* Data,
            size_t      Size) :
            m_Data(Data),
            m_Size(Size)
        {
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID Riid,
            void** Object) override
        {
            if (!Object)
            {
                return E_POINTER;
            }

            if (Riid == IID_IUnknown)
            {
                *Object = static_cast<IUnknown*>(this);
                AddRef();
                return S_OK;
            }

            if (Riid == __uuidof(IDxcBlob))
            {
                *Object = static_cast<IDxcBlob*>(this);
                AddRef();
                return S_OK;
            }

            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return 1;
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            return 1;
        }

        LPVOID STDMETHODCALLTYPE GetBufferPointer() override
        {
            return const_cast<void*>(m_Data);
        }

        SIZE_T STDMETHODCALLTYPE GetBufferSize() override
        {
            return m_Size;
        }

    private:
        const void* m_Data;
        size_t      m_Size;
    };

    //

    class IncludeHandler : public IDxcIncludeHandler
    {
    public:
        IncludeHandler(
            IDxcUtils*          Utils,
            IDxcIncludeHandler* DefaultIncludeHandler,
            StringU8View        IncludeDirectory) :
            m_Utils(Utils),
            m_DefaultIncludeHandler(DefaultIncludeHandler),
            m_IncludeDirectory(IncludeDirectory)
        {
        }

        ~IncludeHandler()
        {
            for (auto& Handle : m_TextFileHandle)
            {
                Asset::Manager::RequestUnload(Handle);
            }
        }

        HRESULT STDMETHODCALLTYPE LoadSource(
            _In_ LPCWSTR                             FileName,
            _COM_Outptr_result_maybenull_ IDxcBlob** IncludeSourceBlob) override
        {
            *IncludeSourceBlob = nullptr;
            auto Iter          = m_LoadedFiles.find(FileName);
            if (Iter != m_LoadedFiles.end())
            {
                *IncludeSourceBlob = Iter->second.Get();
                (*IncludeSourceBlob)->AddRef();
                return S_OK;
            }

            if (FileName[0] == '.' && FileName[1] == '/')
            {
                FileName += 2;
            }

            size_t GoBackCount = 0;
            while (FileName && FileName[0] == '.' && FileName[1] == '.' && FileName[2] == '/')
            {
                FileName += 3;
                GoBackCount++;
            }

            if (!FileName) [[unlikely]]
            {
                return E_FAIL;
            }

            auto FileNameStr = StringUtils::Transform<StringU8>(FileName);

            // At first, we will find all shaders in Engine/Shaders/*, and GoBackCount could be 0 or 1 (depends if the user use ../)
            uint32_t GoBackStart = 1;

            for (std::filesystem::path Path : {
                     "Engine/Shaders",
                     m_IncludeDirectory.empty() ? m_IncludeDirectory.data() : nullptr })
            {
                if (Path.empty()) [[unlikely]]
                {
                    GoBackStart = 0;
                    continue;
                }

                bool ValidPath = true;
                for (size_t i = GoBackStart; i < GoBackCount; i++)
                {
                    if (!Path.has_parent_path())
                    {
                        ValidPath = false;
                        break;
                    }
                    Path = Path.parent_path();
                }

                GoBackStart = 0;

                if (!ValidPath)
                {
                    continue;
                }

                StringU8 PathToAsset = StringUtils::Format(
                    "{}/{}",
                    Path.string(),
                    FileNameStr);

                auto& Handle = m_TextFileHandle.emplace_back(Asset::Storage::FindAsset(PathToAsset, true, true).second);
                if (Handle.is_nil()) [[unlikely]]
                {
                    m_TextFileHandle.pop_back();
                    continue;
                }

                auto TextFile = std::dynamic_pointer_cast<Asset::TextFileAsset>(Asset::Manager::Load(Handle));
                if (!TextFile) [[unlikely]]
                {
                    m_TextFileHandle.pop_back();
                    continue;
                }

                auto& SourceCode = TextFile->Get();

                WinAPI::ComPtr<IDxcBlobEncoding> ShaderCodeBlob;
                HRESULT                          Res = m_Utils->CreateBlobFromPinned(
                    SourceCode.data(),
                    uint32_t(SourceCode.size()),
                    DXC_CP_UTF8,
                    &ShaderCodeBlob);

                if (SUCCEEDED(Res))
                {
                    *IncludeSourceBlob = ShaderCodeBlob.Get();
                    (*IncludeSourceBlob)->AddRef();
                    m_LoadedFiles.emplace(FileName, std::move(ShaderCodeBlob));
                    return S_OK;
                }
                else
                {
                    m_TextFileHandle.pop_back();
                }
            }

            return E_FAIL;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID Riid,
            void** Object) override
        {
            return m_DefaultIncludeHandler->QueryInterface(Riid, Object);
        }

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return 1;
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            return 1;
        }

    private:
        IDxcUtils*          m_Utils;
        IDxcIncludeHandler* m_DefaultIncludeHandler;
        StringU8View        m_IncludeDirectory;

        std::list<Asset::Handle>                           m_TextFileHandle;
        std::map<String, WinAPI::ComPtr<IDxcBlobEncoding>> m_LoadedFiles;
    };

    //

    std::unique_ptr<uint8_t[]> Dx12ShaderCompiler::Compile(
        StringU8View             IncludeDirectory,
        StringU8View             SourceCode,
        const ShaderCompileDesc& Desc,
        size_t&                  DataSize)
    {
        WinAPI::ComPtr<IDxcBlobEncoding> ShaderCodeBlob;
        ThrowIfFailed(m_Utils->CreateBlobFromPinned(
            SourceCode.data(),
            uint32_t(SourceCode.size()),
            DXC_CP_UTF8,
            &ShaderCodeBlob));

        std::vector<const wchar_t*> Options;
#ifndef NEON_DIST
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

        Options.emplace_back(DXC_ARG_OPTIMIZATION_LEVEL3);

        Options.emplace_back(DXC_ARG_ALL_RESOURCES_BOUND);
        Options.emplace_back(STR("-HV 2021"));

        Options.emplace_back(STR("-E"));
        const wchar_t* EntryPoint = nullptr;
        switch (Desc.Stage)
        {
        case ShaderStage::Compute:
            EntryPoint = STR("CS_Main");
            break;
        case ShaderStage::Vertex:
            EntryPoint = STR("VS_Main");
            break;
        case ShaderStage::Hull:
            EntryPoint = STR("HS_Main");
            break;
        case ShaderStage::Domain:
            EntryPoint = STR("DS_Main");
            break;
        case ShaderStage::Geometry:
            EntryPoint = STR("GS_Main");
            break;
        case ShaderStage::Pixel:
            EntryPoint = STR("PS_Main");
            break;
        }
        Options.emplace_back(EntryPoint);

        Options.emplace_back(STR("-T"));
        String Model = GetShaderModel(Desc.Stage, Desc.Profile);
        Options.emplace_back(Model.c_str());

        std::vector<String> MacrosCombined;
        MacrosCombined.reserve(Desc.Macros.Size());
        Options.reserve(Options.size() + Desc.Macros.Size());

        for (auto& [Key, Value] : Desc.Macros.Get())
        {
            MacrosCombined.emplace_back(StringUtils::Format(
                STR("{}{}"),
                Key,
                Value.empty() ? STR("=1") : (L'=' + Value)));
            Options.emplace_back(STR("-D"));
            Options.emplace_back(MacrosCombined.back().c_str());
        }

        DxcBuffer Buffer{
            .Ptr      = ShaderCodeBlob->GetBufferPointer(),
            .Size     = ShaderCodeBlob->GetBufferSize(),
            .Encoding = DXC_CP_ACP
        };

        WinAPI::ComPtr<IDxcResult> Result;
        {
            IncludeHandler Handler(
                m_Utils.Get(),
                m_DefaultIncludeHandler.Get(),
                IncludeDirectory);

            ThrowIfFailed(m_Compiler->Compile(
                &Buffer,
                Options.data(),
                uint32_t(Options.size()),
                &Handler,
                IID_PPV_ARGS(&Result)));

            WinAPI::ComPtr<IDxcBlobUtf8> Error;
            if (SUCCEEDED(Result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&Error), nullptr)) &&
                (Error &&
                 Error->GetStringLength() > 0))
            {
                size_t StrLen = Error->GetStringLength();
                NEON_ERROR_TAG("ShaderCompiler", StringU8(Error->GetStringPointer(), StrLen));
                return nullptr;
            }
        }

        WinAPI::ComPtr<IDxcBlob> Data;
        ThrowIfFailed(Result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&Data), nullptr));

        // Add validation
        {
            WinAPI::ComPtr<IDxcOperationResult> OperationResult;
            ThrowIfFailed(m_Validator->Validate(
                Data.Get(),
                DxcValidatorFlags_InPlaceEdit,
                &OperationResult));

            HRESULT Status;
            ThrowIfFailed(OperationResult->GetStatus(&Status));

            if (FAILED(Status))
            {
                WinAPI::ComPtr<IDxcBlobEncoding> Error;
                WinAPI::ComPtr<IDxcBlobUtf8>     ErrorUtf8;
                OperationResult->GetErrorBuffer(&Error);
                m_Utils->GetBlobAsUtf8(Error.Get(), &ErrorUtf8);

                NEON_ERROR_TAG("ShaderCompiler", StringU8(ErrorUtf8->GetStringPointer(), ErrorUtf8->GetBufferSize()));
                return nullptr;
            }
            else
            {
                ThrowIfFailed(OperationResult->GetResult(&Data));
            }
        }

        uint8_t* CompiledShaderCode = static_cast<uint8_t*>(Data->GetBufferPointer());

        size_t CodeSize = Data->GetBufferSize();
        auto   CodePtr  = std::make_unique<uint8_t[]>(CodeSize);

        std::copy(CompiledShaderCode, CompiledShaderCode + CodeSize, CodePtr.get());

        DataSize = CodeSize;
        return CodePtr;
    }

    void Dx12ShaderCompiler::ReflectLayout(
        const void*        ShaderCode,
        size_t             ByteLength,
        ShaderInputLayout& Layout,
        bool               IsOutput)
    {
        WinAPI::ComPtr<IDxcContainerReflection> Reflection;
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(Reflection.GetAddressOf())));

        ReflectionBlob Blob(ShaderCode, ByteLength);
        ThrowIfFailed(Reflection->Load(&Blob));

        uint32_t DxilPart;
        ThrowIfFailed(Reflection->FindFirstPartKind(DXC_PART_DXIL, &DxilPart));

        WinAPI::ComPtr<ID3D12ShaderReflection> ShaderReflection;
        ThrowIfFailed(Reflection->GetPartReflection(DxilPart, IID_PPV_ARGS(&ShaderReflection)));

        ReflectLayout(
            ShaderReflection.Get(),
            Layout,
            IsOutput);
    }

    void Dx12ShaderCompiler::ReflectLayout(
        ID3D12ShaderReflection* ShaderReflection,
        ShaderInputLayout&      Layout,
        bool                    IsOutput)
    {
        D3D12_SHADER_DESC ShaderDesc;
        ThrowIfFailed(ShaderReflection->GetDesc(&ShaderDesc));

        uint32_t ParamsCount = IsOutput ? ShaderDesc.OutputParameters : ShaderDesc.InputParameters;

        D3D12_SIGNATURE_PARAMETER_DESC SigParam;

        for (uint32_t i = 0; i < ParamsCount; i++)
        {
            if (IsOutput)
                ShaderReflection->GetOutputParameterDesc(i, &SigParam);
            else
                ShaderReflection->GetInputParameterDesc(i, &SigParam);

            // Ignore system values semantics (SV_*)
            if (SigParam.SemanticName[0] == 'S' && SigParam.SemanticName[1] == 'V' && SigParam.SemanticName[2] == '_')
                continue;

            EResourceFormat Format;

            auto SelectFrom = [](D3D_REGISTER_COMPONENT_TYPE Type, auto UIntType, auto IntType, auto FloatType)
            {
                switch (Type)
                {
                case D3D_REGISTER_COMPONENT_UINT32:
                    return UIntType;
                case D3D_REGISTER_COMPONENT_SINT32:
                    return IntType;
                case D3D_REGISTER_COMPONENT_FLOAT32:
                    return FloatType;
                default:
                    std::unreachable();
                    break;
                }
            };

            if (SigParam.Mask & (1 << 3))
            {
                Format = SelectFrom(
                    SigParam.ComponentType,
                    EResourceFormat::R32G32B32A32_UInt,
                    EResourceFormat::R32G32B32A32_SInt,
                    EResourceFormat::R32G32B32A32_Float);
            }
            else if (SigParam.Mask & (1 << 2))
            {
                Format = SelectFrom(
                    SigParam.ComponentType,
                    EResourceFormat::R32G32B32_UInt,
                    EResourceFormat::R32G32B32_SInt,
                    EResourceFormat::R32G32B32_Float);
            }
            else if (SigParam.Mask & (1 << 1))
            {
                Format = SelectFrom(
                    SigParam.ComponentType,
                    EResourceFormat::R32G32_UInt,
                    EResourceFormat::R32G32_SInt,
                    EResourceFormat::R32G32_Float);
            }
            else
            {
                Format = SelectFrom(
                    SigParam.ComponentType,
                    EResourceFormat::R32_UInt,
                    EResourceFormat::R32_SInt,
                    EResourceFormat::R32_Float);
            }

            StringU8 Name = SigParam.SemanticName;
            if (SigParam.SemanticIndex != 0)
            {
                Name += "#" + std::to_string(SigParam.SemanticIndex);
            }
            Layout.emplace_back(std::move(Name), Format);
        }
    }

    WinAPI::ComPtr<ID3D12ShaderReflection> Dx12ShaderCompiler::GetReflection(
        const void* ShaderCode,
        size_t      ByteLength)
    {
        WinAPI::ComPtr<IDxcContainerReflection> Reflection;
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(Reflection.GetAddressOf())));

        ReflectionBlob Blob(ShaderCode, ByteLength);
        ThrowIfFailed(Reflection->Load(&Blob));

        uint32_t DxilPart;
        ThrowIfFailed(Reflection->FindFirstPartKind(DXC_PART_DXIL, &DxilPart));

        WinAPI::ComPtr<ID3D12ShaderReflection> ShaderReflection;
        ThrowIfFailed(Reflection->GetPartReflection(DxilPart, IID_PPV_ARGS(&ShaderReflection)));

        return ShaderReflection;
    }

    //

    String Dx12ShaderCompiler::GetShaderModel(
        ShaderStage   Stage,
        ShaderProfile Profile)
    {
        String Model;

        switch (Stage)
        {
        case ShaderStage::Compute:
            Model = STR("cs");
            break;
        case ShaderStage::Vertex:
            Model = STR("vs");
            break;
        case ShaderStage::Geometry:
            Model = STR("gs");
            break;
        case ShaderStage::Hull:
            Model = STR("hs");
            break;
        case ShaderStage::Domain:
            Model = STR("ds");
            break;
        case ShaderStage::Pixel:
            Model = STR("ps");
            break;
        default:
            std::unreachable();
        }

        switch (Profile)
        {
        case ShaderProfile::SP_6_0:
            Model += STR("_6_0");
            break;
        case ShaderProfile::SP_6_1:
            Model += STR("_6_1");
            break;
        case ShaderProfile::SP_6_2:
            Model += STR("_6_2");
            break;
        case ShaderProfile::SP_6_3:
            Model += STR("_6_3");
            break;
        case ShaderProfile::SP_6_4:
            Model += STR("_6_4");
            break;
        case ShaderProfile::SP_6_5:
            Model += STR("_6_5");
            break;
        case ShaderProfile::SP_6_6:
            Model += STR("_6_6");
            break;
        default:
            std::unreachable();
        }

        return Model;
    }
} // namespace Neon::RHI