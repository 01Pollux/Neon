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
        [[nodiscard]] const Ptr<RHI::IShader>& GetShader();

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

            size_t QuerySize(
                const Ptr<IAssetResource>& Resource) override;

            Ptr<IAssetResource> Load(
                std::istream& Stream,
                size_t        DataSize) override;

            void Save(
                const Ptr<IAssetResource>& Resource,
                std::ostream&              Stream,
                size_t                     DataSize) override;
        };

    private:
        Ptr<RHI::IShader> m_Shader;
    };
} // namespace Neon::Asset