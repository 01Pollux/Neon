#pragma once

#include <Asset/Asset.hpp>
#include <Asset/Handler.hpp>

namespace Neon::RHI
{
    class IRootSignature;
}

namespace Neon::Asset
{
    class RootSignatureAsset : public IAsset
    {
        class Handler;

    public:
        RootSignatureAsset(
            const Ptr<RHI::IRootSignature>& RootSignature,
            const Handle&                   AssetGuid,
            StringU8                        Path);

        /// <summary>
        /// Get current root signature loaded in this asset.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IRootSignature>& GetRootSignature() const;

        /// <summary>
        /// Set root signature to this asset.
        /// </summary>
        [[nodiscard]] void SetRootSignature(
            const Ptr<RHI::IRootSignature>& RootSignature);

    private:
        Ptr<RHI::IRootSignature> m_RootSignature;
    };
} // namespace Neon::Asset