#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/ShaderCompiler.hpp>
#include <RHI/Resource/MappedBuffer.hpp>

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

    class IncludeHandler : public IDxcIncludeHandler
    {
    public:
        IncludeHandler(
            const std::map<String, String>& IncludeFiles,
            IDxcUtils*                      Utils) :
            m_Utils(Utils),
            m_IncludeFiles(IncludeFiles)
        {
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void** ppvObject) override
        {
            if (ppvObject == nullptr)
            {
                return E_POINTER;
            }

            if (riid == IID_IUnknown)
            {
                *ppvObject = static_cast<IUnknown*>(this);
                AddRef();
                return S_OK;
            }

            if (riid == __uuidof(IDxcIncludeHandler))
            {
                *ppvObject = static_cast<IDxcIncludeHandler*>(this);
                AddRef();
                return S_OK;
            }

            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return ++m_RefCount;
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            ULONG Result = --m_RefCount;
            if (Result == 0)
            {
                delete this;
            }
            return Result;
        }

        HRESULT STDMETHODCALLTYPE LoadSource(
            LPCWSTR    FileName,
            IDxcBlob** OutIncludeSource) override
        {
            if (OutIncludeSource == nullptr)
            {
                return E_POINTER;
            }

            auto FileIter = m_IncludeFiles.find(FileName);
            if (FileIter == m_IncludeFiles.end())
            {
                return E_FAIL;
            }

            Win32::ComPtr<IDxcBlobEncoding> IncludeSource;
            ThrowIfFailed(m_Utils->CreateBlobFromPinned(
                FileIter->second.c_str(),
                uint32_t(FileIter->second.size()),
                DXC_CP_ACP,
                &IncludeSource));

            *OutIncludeSource = IncludeSource.Detach();
            return S_OK;
        }

    private:
        ULONG      m_RefCount = 0;
        IDxcUtils* m_Utils;

        const std::map<String, String>& m_IncludeFiles;
    };

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
            REFIID riid,
            void** ppvObject) override
        {
            if (ppvObject == nullptr)
            {
                return E_POINTER;
            }

            if (riid == IID_IUnknown)
            {
                *ppvObject = static_cast<IUnknown*>(this);
                AddRef();
                return S_OK;
            }

            if (riid == __uuidof(IDxcBlob))
            {
                *ppvObject = static_cast<IDxcBlob*>(this);
                AddRef();
                return S_OK;
            }

            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return ++m_RefCount;
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            ULONG Result = --m_RefCount;
            if (Result == 0)
            {
                delete this;
            }
            return Result;
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
        ULONG m_RefCount = 0;

        const void* m_Data;
        size_t      m_Size;
    };

    //

    IShader::ByteCode Dx12ShaderCompiler::Compile(
        const ShaderCompileDesc& Desc)
    {
        Win32::ComPtr<IDxcBlobEncoding> ShaderCodeBlob;
        ThrowIfFailed(m_Utils->CreateBlobFromPinned(
            Desc.SourceCode.c_str(),
            uint32_t(Desc.SourceCode.size()),
            DXC_CP_ACP,
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
            .Encoding = DXC_CP_ACP
        };

        Win32::ComPtr<IDxcResult> Result;
        {
            Win32::ComPtr<IncludeHandler> Handler;
            Handler.Attach(NEON_NEW IncludeHandler(Desc.IncludeFiles, m_Utils.Get()));

            ThrowIfFailed(m_Compiler->Compile(
                &Buffer,
                Options.data(),
                uint32_t(Options.size()),
                Handler.Get(),
                IID_PPV_ARGS(&Result)));
        }

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
        const void*         ShaderCode,
        size_t              ByteLength,
        InputLayoutDesc*    GraphicsLayout,
        MBuffer::RawLayout* Layout,
        bool                IsOutput)
    {
        Win32::ComPtr<IDxcContainerReflection> Reflection;
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(Reflection.GetAddressOf())));

        Win32::ComPtr<ReflectionBlob> Blob;
        Blob.Attach(NEON_NEW ReflectionBlob(ShaderCode, ByteLength));
        ThrowIfFailed(Reflection->Load(Blob.Get()));

        uint32_t DxilPart;
        ThrowIfFailed(Reflection->FindFirstPartKind(MAKEFOURCC('D', 'X', 'I', 'L'), &DxilPart));

        Win32::ComPtr<ID3D12ShaderReflection> ShaderReflection;
        ThrowIfFailed(Reflection->GetPartReflection(DxilPart, IID_PPV_ARGS(&ShaderReflection)));

        ReflectInputLayout(
            ShaderReflection.Get(),
            GraphicsLayout,
            Layout,
            IsOutput);
    }

    void Dx12ShaderCompiler::ReflectInputLayout(
        ID3D12ShaderReflection* ShaderReflection,
        InputLayoutDesc*        GraphicsLayout,
        MBuffer::RawLayout*     Layout,
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

            D3D12_INPUT_ELEMENT_DESC Element{};

            MBuffer::Type Type;
            DXGI_FORMAT   Format;

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
                    DXGI_FORMAT_R32G32B32A32_UINT,
                    DXGI_FORMAT_R32G32B32A32_SINT,
                    DXGI_FORMAT_R32G32B32A32_FLOAT);
                Type = SelectFrom(
                    SigParam.ComponentType,
                    MBuffer::Type::UInt4,
                    MBuffer::Type::Int4,
                    MBuffer::Type::Float4);
            }
            else if (SigParam.Mask & (1 << 2))
            {
                Format = SelectFrom(
                    SigParam.ComponentType,
                    DXGI_FORMAT_R32G32B32_UINT,
                    DXGI_FORMAT_R32G32B32_SINT,
                    DXGI_FORMAT_R32G32B32_FLOAT);
                Type = SelectFrom(
                    SigParam.ComponentType,
                    MBuffer::Type::UInt3,
                    MBuffer::Type::Int3,
                    MBuffer::Type::Float3);
            }
            else if (SigParam.Mask & (1 << 1))
            {
                Format = SelectFrom(
                    SigParam.ComponentType,
                    DXGI_FORMAT_R32G32_UINT,
                    DXGI_FORMAT_R32G32_SINT,
                    DXGI_FORMAT_R32G32_FLOAT);
                Type = SelectFrom(
                    SigParam.ComponentType,
                    MBuffer::Type::UInt2,
                    MBuffer::Type::Int2,
                    MBuffer::Type::Float2);
            }
            else
            {
                Format = SelectFrom(
                    SigParam.ComponentType,
                    DXGI_FORMAT_R32_UINT,
                    DXGI_FORMAT_R32_SINT,
                    DXGI_FORMAT_R32_FLOAT);
                Type = SelectFrom(
                    SigParam.ComponentType,
                    MBuffer::Type::UInt,
                    MBuffer::Type::Int,
                    MBuffer::Type::Float);
            }

            Element.Format = Format;

            if (GraphicsLayout)
            {
                Element.SemanticName      = SigParam.SemanticName;
                Element.SemanticIndex     = SigParam.SemanticIndex;
                Element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

                // GraphicsLayout->AppendElement(Element);
            }
            if (Layout)
            {
                StringU8 Name = SigParam.SemanticName;
                if (SigParam.SemanticIndex != 0)
                {
                    Name += std::to_string(SigParam.SemanticIndex);
                }
                Layout->Append(Type, std::move(Name));
            }
        }
    }

    String Dx12ShaderCompiler::GetShaderModel(
        ShaderStage   Stage,
        ShaderProfile Profile)
    {
        String Model;

        switch (Stage)
        {
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
        }

        return Model;
    }
} // namespace Neon::RHI