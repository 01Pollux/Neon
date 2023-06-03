#pragma once

#include <Resource/Asset.hpp>
#include <Resource/Handler.hpp>

namespace Neon::RHI
{
    class IRootSignature;
}

namespace Neon::Asset
{
    class RootSignatureAsset : public IAssetResource
    {
    public:
        RootSignatureAsset(
            const Ptr<RHI::IRootSignature>& RootSignature = nullptr);

        /// <summary>
        /// Get current root signature loaded in this asset.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IRootSignature>& GetRootSignature() const;

        /// <summary>
        /// Set root signature to this asset.
        /// </summary>
        [[nodiscard]] void SetRootSignature(
            const Ptr<RHI::IRootSignature>& Shader);

    public:
        class Handler : public IAssetResourceHandler
        {
        public:
            bool CanCastTo(
                const Ptr<IAssetResource>& Resource) override;

            Ptr<IAssetResource> Load(
                IO::BinaryStreamReader Stream,
                size_t                 DataSize) override;

            void Save(
                const Ptr<IAssetResource>& Resource,
                IO::BinaryStreamWriter     Stream) override;
        };

    private:
        Ptr<RHI::IRootSignature> m_RootSignature;
    };
} // namespace Neon::Asset