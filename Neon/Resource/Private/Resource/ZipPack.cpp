#include <ResourcePCH.hpp>
#include <Private/Resource/ZipPack.hpp>
#include <Resource/Handler.hpp>

#include <future>
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
    ZipAssetPack::ZipAssetPack(
        uint16_t PackId) :
        IAssetPack(PackId)
    {
    }

    void ZipAssetPack::Import(
        const StringU8& FilePath)
    {
        m_AssetsInfo.clear();
        m_LoadedAssets.clear();

        OpenFile(FilePath);

        try
        {
            if (!ReadFile())
            {
                m_FileView = {};
                NEON_ERROR("Resource", "Invalid pack file '{}'", FilePath);
            }
        }
        catch (const std::exception& Exception)
        {
            NEON_ERROR("Resource", "Exception caught while trying to read file '{}', ({})", FilePath, Exception.what());
        }
    }

    void ZipAssetPack::Export(
        const AssetResourceHandlers& Handlers,
        const StringU8&              FilePath)
    {
        size_t FileSize = 0;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            if (!Handlers.Get(Info.LoaderId))
            {
                NEON_WARNING("Resource", "Tried to export a resource '{}' with an unknown handler", buuid::to_string(Handle));
                continue;
            }

            Info.Offset = FileSize;
            FileSize += Info.Size;
        }

        bio::mapped_file_params Params(FilePath);
        Params.flags         = bio::mapped_file::readwrite;
        Params.new_file_size = FileSize;

        m_FileView = bio::mapped_file(Params);
        WriteFile(Handlers);
        OpenFile(FilePath);
    }

    //

    Ref<IAssetResource> ZipAssetPack::Load(
        const AssetResourceHandlers& Handlers,
        const AssetHandle&           Handle)
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
        const AssetResourceHandlers& Handlers,
        const AssetHandle&           Handle,
        const Ptr<IAssetResource>&   Resource)
    {
        for (auto& [LoaderId, Handler] : Handlers.Get())
        {
            if (Handler->CanCastTo(Resource))
            {
                if (std::exchange(m_LoadedAssets[Handle], Resource) != Resource)
                {
                    auto& Info    = m_AssetsInfo[Handle];
                    Info.LoaderId = LoaderId;
                    Info.Size     = Handler->QuerySize(Resource);
                }
                return;
            }
        }

        NEON_WARNING("Resource", "No handler support resource '{}'", buuid::to_string(Handle));
    }

    //

    Ref<IAssetResource> ZipAssetPack::LoadAsset(
        const AssetResourceHandlers& Handlers,
        const AssetHandle&           Handle,
        StringU8&                    ErrorText)
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

        auto Handler = Handlers.Get(Info.LoaderId);
        if (!Handler)
        {
            ErrorText = "Tried loading asset '{}' with handler that doesn't exists";
            return Asset;
        }

        return Handler->Load(DataPtr, Info.Size);
    }

    //

    struct AssetPackHeader
    {
        static constexpr uint16_t DefaultSignature = 0xF139;
        static constexpr uint8_t  LatestVersion    = 0;

        uint16_t      Signature;
        uint8_t       Version;
        SHA256::Bytes Hash;
        uint16_t      NumberOfResources;
    };

    struct AssetPackSection
    {
        static constexpr uint16_t DefaultSignature = AssetPackHeader::DefaultSignature;

        uint16_t               Signature;
        AssetHandle            Handle;
        ZipAssetPack::PackInfo PackInfo;
    };

    //

    void ZipAssetPack::OpenFile(
        const StringU8& FilePath)
    {
        bio::mapped_file_params Params(FilePath);
        Params.flags = bio::mapped_file::readonly;

        m_FileView = bio::mapped_file(Params);
    }

    bool ZipAssetPack::ReadFile()
    {
        return Header_ReadSections() && Header_ReadBody();
    }

    bool ZipAssetPack::Header_ReadSections()
    {
        bio::stream<bio::mapped_file> Stream(m_FileView);

        AssetPackHeader Header;
        Stream.read(std::bit_cast<char*>(&Header), sizeof(Header));

        if (Header.Signature != AssetPackHeader::DefaultSignature ||
            Header.NumberOfResources == 0)
        {
            return false;
        }

        AssetPackSection Section;
        for (uint16_t i = 0; i < Header.NumberOfResources; i++)
        {
            Stream >> Section.Signature;
            if (Header.Signature != Section.Signature)
            {
                NEON_INFO("Resource", "Invalid resource signature");
                return false;
            }
            Stream >> Section.Handle;

            auto& Info = m_AssetsInfo[Section.Handle];
            if (Info.Size)
            {
                NEON_INFO("Resource", "Duplicate resource '{}'", buuid::to_string(Section.Handle));
                return false;
            }

            Stream.read(std::bit_cast<char*>(&Info), sizeof(Info));
            // Info = Section.PackInfo;
        }

        return true;
    }

    bool ZipAssetPack::Header_ReadBody()
    {
        bool Succeeded = true;

        std::vector<std::future<void>> AssetsValidators;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            AssetsValidators.emplace_back(
                std::async(
                    [this, &Handle, &Info, &Succeeded]
                    {
                        bio::stream<bio::mapped_file> Stream(m_FileView);

                        SHA256 Sha256;
                        char   Byte;

                        Stream.seekg(Info.Offset);
                        for (size_t i = 0; i < Info.Size && Succeeded; i++)
                        {
                            Stream >> Byte;
                            Sha256.Append(Byte);
                        }

                        if (Succeeded && Sha256.Digest() != Info.Hash)
                        {
                            NEON_INFO("Resource", "Invalid resource Sha256 for '{}'", buuid::to_string(Handle));
                            Succeeded = false;
                        }
                    }));
        }

        AssetsValidators.clear();
        return Succeeded;
    }

    //

    void ZipAssetPack::WriteFile(
        const AssetResourceHandlers& Handlers)
    {
        Header_WriteSections();
        Header_WriteBody(Handlers);

        bio::stream<bio::mapped_file> Stream(m_FileView);

        AssetPackSection Section;

        AssetPackHeader Header{
            .Signature         = AssetPackHeader::DefaultSignature,
            .Version           = AssetPackHeader::LatestVersion,
            .NumberOfResources = uint16_t(m_AssetsInfo.size())
        };
        Stream.write(std::bit_cast<const char*>(&Header), sizeof(Header));
    }

    void ZipAssetPack::Header_WriteSections()
    {
        bio::stream<bio::mapped_file> Stream(m_FileView);

        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            Stream << AssetPackSection::DefaultSignature;
            Stream << Handle;
            Stream.write(std::bit_cast<const char*>(&Info), sizeof(Info));
        }
    }

    void ZipAssetPack::Header_WriteBody(
        const AssetResourceHandlers& Handlers)
    {
        std::vector<std::future<void>> AssetsWriter;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            AssetsWriter.emplace_back(
                std::async(
                    [this, &Handle, &Info, &Handlers]
                    {
                        auto  Handler  = Handlers.Get(Info.LoaderId);
                        auto& Resource = m_LoadedAssets[Handle];

                        uint8_t* Data = std::bit_cast<uint8_t*>(m_FileView.data() + Info.Offset);
                        Handler->Save(Resource, Data, Info.Size);

                        SHA256 Sha256;
                        Sha256.Append(Data, Info.Size);
                        Info.Hash = Sha256.Digest();
                    }));
        }
    }
} // namespace Neon::Asset
