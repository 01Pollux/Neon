#pragma once

#include <Asset/Asset.hpp>
#include <RHI/Shader.hpp>

#include <Asset/Asset.hpp>
#include <Asset/Handler.hpp>
#include <RHI/Resource/Resource.hpp>

namespace Neon::Asset
{
    class ShaderAsset : public IAsset
    {
        class Handler;

    public:
        ShaderAsset(
            StringU8      ShaderCode,
            const Handle& AssetGuid,
            StringU8      Path);

        /// <summary>
        /// Load the shader cache from settings
        /// </summary>
        UPtr<RHI::IShader> LoadShader(
            const RHI::ShaderCompileDesc& Desc);

        /// <summary>
        /// Clear the shader cache
        /// </summary>
        void ClearCache();

    private:
        /// <summary>
        /// Open the shader cache file
        /// </summary>
        void OpenCacheFile(
            bool Reset = false);

    private:
        std::fstream m_ShaderCacheFile;
        size_t       m_ShaderCacheSize = 0;
        StringU8     m_ShaderCode;
    };
} // namespace Neon::Asset