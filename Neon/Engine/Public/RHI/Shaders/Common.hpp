#pragma once

#include <Asset/Asset.hpp>
#include <Asset/Types/Shader.hpp>
#include <RHI/Shaders/CommonGuids.hpp>

namespace Neon::RHI::Shaders
{
    /// <summary>
    /// Helper class to load shaders async and discard them at the end of the scope.
    /// </summary>
    class GlobalShader
    {
    private:
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

    public:
        GlobalShader(
            const Asset::Handle& Handle);

        [[nodiscard]] auto operator->() const
        {
            return m_Shader.Get();
        }

        [[nodiscard]] auto Get() const
        {
            return m_Shader.Get();
        }

    private:
        ShaderAssetTaskPtr m_Shader;
    };
} // namespace Neon::RHI