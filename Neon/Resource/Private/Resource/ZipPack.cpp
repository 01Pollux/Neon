#include <ResourcePCH.hpp>
#include <Private/Resource/ZipPack.hpp>
#include <Resource/Handler.hpp>

#include <Core/SHA256.hpp>

#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#include <Log/Logger.hpp>

namespace bio   = boost::iostreams;
namespace buuid = boost::uuids;

namespace Neon::Asset
{
    void ZipAssetPack::Import(
        const StringU8& FilePath)
    {
        m_AssetsInfo.clear();
        m_LoadedAssets.clear();

        bio::mapped_file_params Params(FilePath);
        Params.flags = bio::mapped_file::readonly;

        m_FileView = bio::mapped_file(Params);

        try
        {
            if (!ReadHeader())
            {
                m_FileView = {};
                NEON_ERROR("Resource", "Invalid pack file '{}'.", FilePath);
            }
        }
        catch (const std::exception& Exception)
        {
            NEON_ERROR("Resource", "Exception caught while trying to read file '{}', ({}).", FilePath, Exception.what());
        }
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

        NEON_INFO_TAG("Resource", "Loading asset: {}", buuid::to_string(Handle));

        StringU8 ErrorText;
        auto     Asset = LoadAsset(Handlers, Handle, ErrorText);

        if (ErrorText.empty())
        {
            NEON_WARNING_TAG("Resource", ErrorText, buuid::to_string(Handle));
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

    //

    struct AssetPackHeader
    {
        static constexpr uint16_t DefaultSignature = 0xF139;

        uint16_t      Signature;
        uint8_t       Version;
        SHA256::Bytes Hash;
        uint16_t      NumberOfResources;
    };

    bool ZipAssetPack::ReadHeader()
    {
        bio::stream<bio::mapped_file> Stream(m_FileView);

        AssetPackHeader Header;
        Stream.read(std::bit_cast<char*>(&Header), sizeof(Header));

        if (Header.Signature != AssetPackHeader::DefaultSignature ||
            Header.NumberOfResources == 0)
        {
            return false;
        }

        uint16_t    Signature;
        AssetHandle Uuid;

        for (uint16_t i = 0; i < Header.NumberOfResources; i++)
        {
            Stream >> Signature;
            if (Header.Signature != Signature)
            {
                NEON_INFO("Resource", "Invalid resource signature");
                return false;
            }
            Stream >> Uuid;

            auto& Info = m_AssetsInfo[Uuid];
            if (Info.Size)
            {
                NEON_INFO("Resource", "Duplicate resource '{}'", buuid::to_string(Uuid));
                return false;
            }

            Stream.read(std::bit_cast<char*>(Info.Hash.data()), Info.Hash.size());
            Stream >> Info.LoaderId >> Info.Offset >> Info.Size;
        }

        for (auto& [Uuid, Info] : m_AssetsInfo)
        {
            SHA256 Sha256;
            char   Byte;

            Stream.seekg(Info.Offset);
            for (size_t i = 0; i < Info.Size; i++)
            {
                Stream >> Byte;
                Sha256.Append(Byte);
            }

            if (Sha256.Digest() != Info.Hash)
            {
                NEON_INFO("Resource", "Invalid resource Sha256 for '{}'", buuid::to_string(Uuid));
            }
        }

        return true;
    }
} // namespace Neon::Asset
