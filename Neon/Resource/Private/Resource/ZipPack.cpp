#include <ResourcePCH.hpp>
#include <Private/Resource/ZipPack.hpp>
#include <Resource/Handler.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <Log/Logger.hpp>

namespace Neon::Asset
{
    void ZipAssetPack::Import(
        const StringU8& FilePath)
    {
    }

    void ZipAssetPack::Export(
        const StringU8& FilePath)
    {
    }

    Ref<IAssetResource> ZipAssetPack::Load(
        const AssetResourceHandlerMap& Handlers,
        const AssetHandle&             Handle)
    {
        auto& LoadedAsset = m_LoadedAssets[Handle];

        // Resource already loaded
        if (LoadedAsset)
        {
            return LoadedAsset;
        }

        NEON_INFO_TAG("Resource", "Loading asset: {}", boost::uuids::to_string(Handle));

        StringU8 ErrorText;
        auto     Asset = LoadAsset(Handlers, Handle, ErrorText);

        if (ErrorText.empty())
        {
            NEON_WARNING_TAG("Resource", ErrorText, boost::uuids::to_string(Handle));
        }
        else
        {
            LoadedAsset = Asset.lock();
        }

        return Asset;
    }

    void ZipAssetPack::Save(
        const AssetResourceHandlerMap& Handlers,
        const AssetHandle&             Handle,
        const Ptr<IAssetResource>&     Resource)
    {
        m_LoadedAssets[Handle] = Resource;
    }

    //

    Ref<IAssetResource> ZipAssetPack::LoadAsset(
        const AssetResourceHandlerMap& Handlers,
        const AssetHandle&             Handle,
        StringU8&                      ErrorText)
    {
        Ref<IAssetResource> Asset;

        auto Iter = m_AssetsInfo.find(Handle);
        if (Iter == m_AssetsInfo.end())
        {
            ErrorText = "Tried loading asset '{}' that doesn't exists";
            return Asset;
        }

        auto& Info    = Iter->second;
        auto  DataPtr = std::bit_cast<const uint8_t*>(m_FileView.data()) + Info.Offset;

        auto HandlerIter = Handlers.find(Info.LoaderId);
        if (HandlerIter == Handlers.end())
        {
            ErrorText = "Tried loading asset '{}' with handler that doesn't exists";
            return Asset;
        }

        return HandlerIter->second->Load(DataPtr, Info.Size);
    }
} // namespace Neon::Asset
