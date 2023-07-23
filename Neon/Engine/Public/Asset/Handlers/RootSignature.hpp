#pragma once

#include <Asset/Handler.hpp>
#include <Asset/Types/RootSignature.hpp>

namespace Neon::RHI
{
    class IRootSignature;
}

namespace Neon::Asset
{
    class RootSignatureAsset::Handler : public IAssetHandler
    {
    public:
        bool CanHandle(
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
} // namespace Neon::Asset