#pragma once

#include <Resource/Asset.hpp>
#include <Resource/Handler.hpp>

namespace Neon::RHI
{
    class IShader;
}

namespace Neon::Asset
{
    class ShaderAsset : public IAssetResource
    {
    public:
        ShaderAsset(
            const Ptr<RHI::IShader>& Shader = nullptr);

        /// <summary>
        /// Get shader currently loaded in this asset.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IShader>& GetShader() const;

        /// <summary>
        /// Set shader currently loaded in this asset.
        /// </summary>
        [[nodiscard]] void SetShader(
            const Ptr<RHI::IShader>& Shader);

    public:
        class Handler : public IAssetResourceHandler
        {
        public:
            bool CanCastTo(
                const Ptr<IAssetResource>& Resource) override;

            Ptr<IAssetResource> Load(
                IAssetPack*    Pack,
                IO::InArchive& Archive,
                size_t         DataSize) override;

            void Save(
                IAssetPack*                Pack,
                const Ptr<IAssetResource>& Resource,
                IO::OutArchive&            Archive) override;
        };

    private:
        Ptr<RHI::IShader> m_Shader;
    };
} // namespace Neon::Asset