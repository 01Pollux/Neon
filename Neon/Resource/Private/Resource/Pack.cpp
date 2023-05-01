#include <ResourcePCH.hpp>
#include <Private/Resource/Pack.hpp>
#include <Resource/Handler.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <Log/Logger.hpp>

namespace Neon::Asset
{
    Ref<IAssetResource> AssetPack::Load(
        const AssetResourceHandlerMap& Handlers,
        const AssetHandle&             Handle)
    {
        auto& LoadedAsset = m_LoadedAssets[Handle.Get()];
        if (LoadedAsset)
        {
            return LoadedAsset;
        }

        NEON_INFO_TAG("Resource", "Loading asset: {}", boost::uuids::to_string(Handle.Get()));

        StringU8 ErrorText;
        auto     Asset = LoadAsset(Handlers, Handle, ErrorText);

        if (ErrorText.empty())
        {
            NEON_WARNING_TAG("Resource", ErrorText, boost::uuids::to_string(Handle.Get()));
        }
        else
        {
            LoadedAsset = Asset.lock();
        }

        return Asset;
    }

    void AssetPack::UnrefAsset(
        const AssetHandle& Handle)
    {
        m_PendingAssets->PendingAssets.erase(Handle.Get());
        if (m_PendingAssets->PendingAssets.empty())
        {
            m_PendingAssets = nullptr;
        }
    }

    //

    Ref<IAssetResource> AssetPack::LoadAsset(
        const AssetResourceHandlerMap& Handlers,
        const AssetHandle&             Handle,
        StringU8&                      ErrorText)
    {
        Ref<IAssetResource> Asset;
        if (!m_PendingAssets)
        {
            ErrorText = "Tried loading asset '{}' that doesn't exists";
            return Asset;
        }

        auto Iter = m_PendingAssets->PendingAssets.find(Handle.Get());
        if (Iter == m_PendingAssets->PendingAssets.end())
        {
            ErrorText = "Tried loading asset '{}' that doesn't exists";
            return Asset;
        }

        auto& Info    = Iter->second;
        auto  DataPtr = std::bit_cast<const uint8_t*>(m_PendingAssets->FileView.data()) + Info.Offset;

        auto HandlerIter = Handlers.find(Info.LoaderId);
        if (HandlerIter == Handlers.end())
        {
            ErrorText = "Tried loading asset '{}' with handler that doesn't exists";
            return Asset;
        }

        return HandlerIter->second->Load(DataPtr, Info.Size);
    }
} // namespace Neon::Asset
