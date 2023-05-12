#include <EnginePCH.hpp>
#include <Renderer/RenderGraph/PassBuilder.hpp>
#include <Renderer/RenderGraph/Storage.hpp>

namespace Renderer::RG
{
    RenderGraphPassResBuilder::RenderGraphPassResBuilder(
        RenderGraphStorage& Storage) :
        m_Storage(Storage)
    {
    }

    void RenderGraphPassResBuilder::CreateBuffer(
        const ResourceId&       Id,
        const ResourceDesc&     Desc,
        RHI::GraphicsBufferType BufferType)
    {
        m_Storage.DeclareBuffer(Id, Desc, BufferType);
        m_ResourcesCreated.emplace(Id);
    }

    void RenderGraphPassResBuilder::CreateTexture(
        const ResourceId&   Id,
        const ResourceDesc& Desc,
        ResourceFlags       Flags)
    {
        m_Storage.DeclareTexture(Id, Desc, Flags);
        m_ResourcesCreated.emplace(Id);
    }

    void RenderGraphPassResBuilder::WriteResource(
        const ResourceViewId&   ViewId,
        const ResourceViewDesc& Desc)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT_MSG(m_Storage.ContainsResource(Id), "Resource doesn't exists");

        m_ResourcesWritten.emplace(Id);

        D3D12_RESOURCE_STATES State;
        D3D12_RESOURCE_FLAGS  AccessFlags;
        switch (ResourceViewDescType(Desc.index()))
        {
        case ResourceViewDescType::UnorderedAccess:
            State       = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            AccessFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            break;
        case ResourceViewDescType::RenderTarget:
            State       = D3D12_RESOURCE_STATE_RENDER_TARGET;
            AccessFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            m_RenderTargets.emplace_back(ViewId);
            break;
        case ResourceViewDescType::DepthStencil:
            State          = D3D12_RESOURCE_STATE_DEPTH_WRITE;
            AccessFlags    = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            m_DepthStencil = ViewId;
            break;

        default:
            std::unreachable();
        }

        SetResourceState(ViewId, State, AccessFlags, false);
        m_Storage.DeclareResourceView(ViewId, Desc);
    }

    void RenderGraphPassResBuilder::WriteDstResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT_MSG(m_Storage.ContainsResource(Id), "Resource doesn't exists");

        m_ResourcesWritten.emplace(Id);
        SetResourceState(ViewId, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_FLAG_NONE, false);
    }

    void RenderGraphPassResBuilder::ReadResource(
        const ResourceViewId&   ViewId,
        ResourceReadAccess      ReadAccess,
        const ResourceViewDesc& Desc)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT_MSG(m_Storage.ContainsResource(Id), "Resource doesn't exists");

        m_ResourcesRead.emplace(Id);

        D3D12_RESOURCE_STATES State = {};
        switch (ReadAccess)
        {
        case ResourceReadAccess::PixelShader:
            State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            break;
        case ResourceReadAccess::NonPixelShader:
            State = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            break;
        case ResourceReadAccess::Any:
            State = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
            break;
        default:
            std::unreachable();
        }

        D3D12_RESOURCE_FLAGS AccessFlags;
        switch (ResourceViewDescType(Desc.index()))
        {
        case ResourceViewDescType::ConstantBuffer:
        case ResourceViewDescType::ShaderResource:
        case ResourceViewDescType::UnorderedAccess:
            AccessFlags = D3D12_RESOURCE_FLAG_NONE;
            break;
        default:
            std::unreachable();
        }

        SetResourceState(ViewId, State, AccessFlags, true);
        m_Storage.DeclareResourceView(ViewId, Desc);
    }

    void RenderGraphPassResBuilder::ReadSrcResource(
        const ResourceViewId& ViewId)
    {
        auto& Id = ViewId.GetResource();
        NEON_ASSERT_MSG(m_Storage.ContainsResource(Id), "Resource doesn't exists");

        m_ResourcesRead.emplace(Id);
        SetResourceState(ViewId, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_FLAG_NONE, false);
    }

    //

    void RenderGraphPassResBuilder::SetResourceState(
        const ResourceViewId& ViewId,
        D3D12_RESOURCE_STATES State,
        D3D12_RESOURCE_FLAGS  AccessFlags,
        bool                  UsedInShader)
    {
        auto& ResourceHandle = m_Storage.GetResource(ViewId.GetResource());
        auto& ResourceDesc   = ResourceHandle.GetDesc();

        ResourceDesc.Flags |= AccessFlags;
        if (UsedInShader)
        {
            ResourceHandle.m_Flags |= ResourceFlags::Internal_UsedInShader;
        }
        m_ResourceStates[ViewId] = State;
    }

    //

    RenderGraphPassRSBuilder::RenderGraphPassRSBuilder(
        RenderGraphStorage& Storage) :
        m_Storage(Storage)
    {
    }

    void RenderGraphPassRSBuilder::CreateRootSignature(
        const ResourceId&             Id,
        const RHI::RootSignatureDesc& Desc)
    {
        NEON_ASSERT_MSG(m_RootSignaturesToLoad.emplace(Id, Desc).second, "Root signature already exists");
    }

    //

    RenderGraphPassShaderBuilder::RenderGraphPassShaderBuilder(
        RenderGraphStorage& Storage) :
        m_Storage(Storage)
    {
    }

    void RenderGraphPassShaderBuilder::LoadShader(
        const ResourceId&                        Id,
        const StringU8&                          Path,
        const RHI::IGraphicsShader::CompileDesc& ShaderWithNoCode)
    {
        NEON_ASSERT_MSG(
            m_ShadersToLoad.emplace(Id, ShaderInfo(Path, ShaderWithNoCode, true)).second,
            "Shader already exists");
    }

    void RenderGraphPassShaderBuilder::LoadShader(
        const ResourceId&                        Id,
        const RHI::IGraphicsShader::CompileDesc& ShaderWithCode)
    {
        NEON_ASSERT_MSG(
            m_ShadersToLoad.emplace(Id, ShaderInfo(StringU8(ShaderWithCode.ShaderCode), ShaderWithCode, false)).second,
            "Shader already exists");
    }

    //

    RenderGraphPassPSOBuilder::RenderGraphPassPSOBuilder(
        RenderGraphStorage&  Storage,
        const ShaderMapType& Shaders) :
        m_Storage(Storage),
        m_LoadedShaders(Shaders)
    {
    }

    ID3D12RootSignature* RenderGraphPassPSOBuilder::GetRootSignature(
        const ResourceId& Id)
    {
        return m_Storage.GetRootSignature(Id).Get();
    }

    RHI::IGraphicsShader* RenderGraphPassPSOBuilder::GetShader(
        const ResourceId& Id) const
    {
        auto Iter = m_LoadedShaders.find(Id);
        NEON_ASSERT_MSG(Iter != m_LoadedShaders.end(), "Shader doesn't exists");
        return Iter->second.get();
    }

    D3D12_SHADER_BYTECODE RenderGraphPassPSOBuilder::GetShaderData(
        const ResourceId& Id)
    {
        return GetShader(Id)->GetData();
    }

    void RenderGraphPassPSOBuilder::CreatePipelineState(
        const ResourceId& Id,
        PipelineStateDesc Desc)
    {
        NEON_ASSERT_MSG(m_PipelineStatesToLoad.emplace(Id, std::move(Desc)).second, "Pipeline state already exists");
    }
} // namespace Renderer::RG