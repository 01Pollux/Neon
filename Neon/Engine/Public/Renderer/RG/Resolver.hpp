#pragma once

#include <Renderer/RG/Pass.hpp>
#include <RHI/Resource/Common.hpp>

namespace Neon
{
    namespace Asset
    {
        struct AssetHandle;
    } // namespace Asset
    namespace RHI
    {
        class IShader;
        class ShaderCompileDesc;
    } // namespace RHI
    namespace Renderer
    {
        class Material;
    } // namespace Renderer
} // namespace Neon

namespace Neon::RG
{
    class IRenderPass::ShaderResolver
    {
    public:
        /// <summary>
        /// Compile a shader from a compile description.
        /// </summary>
        void Load(
            RHI::ShaderCompileDesc& Desc);

        /// <summary>
        /// Load a shader from a compile description.
        /// </summary>
        void Load(
            Ptr<RHI::IShader>& Shader);

        /// <summary>
        /// Load a shader from an asset.
        /// </summary>
        void Load(
            const Asset::AssetHandle& ShaderAsset);
    };

    //

    class IRenderPass::MaterialResolver
    {
    public:
        /// <summary>
        /// Load a material.
        /// </summary>
        void Load(
            const Renderer::Material& Material);

        /// <summary>
        /// Load a material from an asset.
        /// </summary>
        void Load(
            const Asset::AssetHandle& MaterialAsset);
    };

    //

    class IRenderPass::ResourceResolver
    {
    public:
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
            MResourceFlags      Flags);

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
    };
} // namespace Neon::RG