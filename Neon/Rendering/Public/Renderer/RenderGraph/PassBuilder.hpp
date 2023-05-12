#pragma once

#include <Renderer/RenderGraph/ResourceId.hpp>
#include <RHI/Shader.hpp>

namespace Renderer::RG
{
    class RenderGraphStorage;
    using PipelineStateDesc = std::variant<RHI::GraphicsPipelineStateDesc, RHI::ComputePipelineStateDesc>;

    using ShaderMapType = std::map<ResourceId, RHI::IGraphicsShader::UPtr>;

    class RenderGraphPassResBuilder
    {
        friend class RenderGraphBuilder;

    public:
        RenderGraphPassResBuilder(
            RenderGraphStorage& Storage);

        /// <summary>
        /// Create buffer
        /// </summary>
        void CreateBuffer(
            const ResourceId&       Id,
            const ResourceDesc&     Desc,
            RHI::GraphicsBufferType BufferType);

        /// <summary>
        /// Create texture
        /// </summary>
        void CreateTexture(
            const ResourceId&   Id,
            const ResourceDesc& Desc,
            ResourceFlags       Flags);

        /// <summary>
        /// Write resource view
        /// </summary>
        void WriteResource(
            const ResourceViewId&   ViewId,
            const ResourceViewDesc& Desc);

        /// <summary>
        /// Write resource in copy operation
        /// </summary>
        void WriteDstResource(
            const ResourceViewId& ViewId);

        /// <summary>
        /// Read resource view
        /// </summary>
        void ReadResource(
            const ResourceViewId&   ViewId,
            ResourceReadAccess      ReadAccess,
            const ResourceViewDesc& Desc);

        /// <summary>
        /// Read resource in copy operation
        /// </summary>
        void ReadSrcResource(
            const ResourceViewId& ViewId);

    private:
        /// <summary>
        /// Initialize resource to the default state
        /// </summary>
        void SetResourceState(
            const ResourceViewId& Id,
            D3D12_RESOURCE_STATES State,
            D3D12_RESOURCE_FLAGS  AccessFlags,
            bool                  UsedInShader);

    private:
        RenderGraphStorage& m_Storage;

        std::vector<ResourceViewId>   m_RenderTargets;
        std::optional<ResourceViewId> m_DepthStencil;

        std::set<ResourceId> m_ResourcesCreated;
        std::set<ResourceId> m_ResourcesRead;
        std::set<ResourceId> m_ResourcesWritten;

        std::map<ResourceViewId, D3D12_RESOURCE_STATES> m_ResourceStates;
    };

    class RenderGraphPassRSBuilder
    {
        friend class RenderGraphBuilder;

    public:
        RenderGraphPassRSBuilder(
            RenderGraphStorage& Storage);

        /// <summary>
        /// Create root signature
        /// </summary>
        void CreateRootSignature(
            const ResourceId&             Id,
            const RHI::RootSignatureDesc& Desc);

    private:
        RenderGraphStorage& m_Storage;

        std::map<ResourceId, RHI::RootSignatureDesc> m_RootSignaturesToLoad;
    };

    class RenderGraphPassShaderBuilder
    {
        friend class RenderGraphBuilder;

    public:
        RenderGraphPassShaderBuilder(
            RenderGraphStorage& Storage);

        /// <summary>
        /// Load a shader from path to be used later
        /// </summary>
        void LoadShader(
            const ResourceId&                        Id,
            const StringU8&                          Path,
            const RHI::IGraphicsShader::CompileDesc& ShaderWithNoCode);

        /// <summary>
        /// Load a shader to be used later
        /// </summary>
        void LoadShader(
            const ResourceId&                        Id,
            const RHI::IGraphicsShader::CompileDesc& ShaderWithCode);

    private:
        RenderGraphStorage& m_Storage;

        struct ShaderInfo
        {
            StringU8                          CodeOrPath;
            RHI::IGraphicsShader::CompileDesc Desc;
            bool                              IsPath;
        };
        std::map<ResourceId, ShaderInfo> m_ShadersToLoad;
    };

    class RenderGraphPassPSOBuilder
    {
        friend class RenderGraphBuilder;

    public:
        RenderGraphPassPSOBuilder(
            RenderGraphStorage&  Storage,
            const ShaderMapType& Shaders);

        /// <summary>
        /// Get root signature
        /// </summary>
        [[nodiscard]] ID3D12RootSignature* GetRootSignature(
            const ResourceId& Id);

        /// <summary>
        /// Get a loaded shader
        /// </summary>
        [[nodiscard]] RHI::IGraphicsShader* GetShader(
            const ResourceId& Id) const;

        /// <summary>
        /// Get a loaded shader
        /// </summary>
        [[nodiscard]] D3D12_SHADER_BYTECODE GetShaderData(
            const ResourceId& Id);

        /// <summary>
        /// Create pipeline state
        /// </summary>
        void CreatePipelineState(
            const ResourceId& Id,
            PipelineStateDesc Desc);

    private:
        RenderGraphStorage&                     m_Storage;
        const ShaderMapType&                    m_LoadedShaders;
        std::map<ResourceId, PipelineStateDesc> m_PipelineStatesToLoad;
    };
} // namespace Renderer::RG