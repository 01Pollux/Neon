#pragma once

#include <Asset/Asset.hpp>
#include <Asset/Handler.hpp>
#include <RHI/RootSignature.hpp>

namespace Neon::Asset
{
    class RootSignatureAsset : public IAsset
    {
    public:
        class Handler;

        RootSignatureAsset(
            RHI::RootSignatureBuilder RootSignatureBuilder,
            const Handle&             AssetGuid,
            StringU8                  Path);

        /// <summary>
        /// Get current root signature loaded in this asset.
        /// </summary>
        [[nodiscard]] const RHI::RootSignatureBuilder& GetRootSignatureBuilder() const;

        /// <summary>
        /// Get current root signature loaded in this asset.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IRootSignature>& GetRootSignature() const;

    private:
        RHI::RootSignatureBuilder m_RootSignatureBuilder;
        Ptr<RHI::IRootSignature>  m_RootSignature;
    };
} // namespace Neon::Asset