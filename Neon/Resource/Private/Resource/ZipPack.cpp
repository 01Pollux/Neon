#include <ResourcePCH.hpp>
#include <Resource/Packs/ZipPack.hpp>
#include <Resource/Handler.hpp>

#include <future>
#include <filesystem>

#include <Core/SHA256.hpp>
#include <Math/Common.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <Log/Logger.hpp>

namespace bio   = boost::iostreams;
namespace buuid = boost::uuids;

namespace Neon::Asset
{
    ZipAssetPack::ZipAssetPack(
        const AssetResourceHandlers& Handlers) :
        IAssetPack(Handlers)
    {
    }

    ZipAssetPack::~ZipAssetPack()
    {
        if (m_FileStream)
        {
            m_FileStream = {};
            std::filesystem::remove(GetTempFileName());
        }
    }

    void ZipAssetPack::Import(
        const StringU8& FilePath)
    {
        std::lock_guard Lock(m_PackMutex);

        m_AssetsInfo.clear();
        m_LoadedAssets.clear();

        DecompressCopy(FilePath);

        try
        {
            if (!ReadFile())
            {
                m_FileStream = {};
                NEON_ERROR("Resource", "Invalid pack file '{}'", FilePath);
            }
        }
        catch (const std::exception& Exception)
        {
            NEON_ERROR("Resource", "Exception caught while trying to read file '{}', ({})", FilePath, Exception.what());
        }
    }

    void ZipAssetPack::Export(
        const StringU8& FilePath)
    {
        std::lock_guard Lock(m_PackMutex);

        size_t FileSize = SizeOfHeader(m_AssetsInfo.size());
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            if (!m_Handlers.Get(Info.LoaderId))
            {
                NEON_WARNING("Resource", "Tried to export a resource '{}' with an unknown handler", buuid::to_string(Handle));
                continue;
            }

            Info.Offset = FileSize;
            FileSize += Info.Size;
        }

        OpenTempNew();
        WriteFile();

        CompressCopy(FilePath);
        DecompressCopy(FilePath);
    }

    //

    Ref<IAssetResource> ZipAssetPack::Load(
        const AssetHandle& Handle)
    {
        std::lock_guard Lock(m_PackMutex);

        auto& LoadedAsset = m_LoadedAssets[Handle];

        // Resource already loaded
        if (LoadedAsset)
        {
            return LoadedAsset;
        }

        NEON_INFO_TAG("Resource", "Loading asset: {}", buuid::to_string(Handle));

        StringU8 ErrorText;
        auto     Asset = LoadAsset(m_Handlers, Handle, ErrorText);

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
        const AssetHandle&         Handle,
        const Ptr<IAssetResource>& Resource)
    {
        std::lock_guard Lock(m_PackMutex);

        for (auto& [LoaderId, Handler] : m_Handlers.Get())
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

        auto& Info = Iter->second;
        m_FileStream.seekg(Info.Offset);

        auto Handler = Handlers.Get(Info.LoaderId);
        if (!Handler)
        {
            ErrorText = "Tried loading asset '{}' with handler that doesn't exists";
            return Asset;
        }

        return Handler->Load(m_FileStream, Info.Size);
    }

    //

    StringU8 ZipAssetPack::GetTempFileName() const
    {
        return StringUtils::Format(
            "{}_{}.rnp",
            std::filesystem::temp_directory_path().string(),
            static_cast<const void*>(this));
    }

    void ZipAssetPack::OpenTempNew()
    {
        if (m_FileStream.is_open())
        {
            m_FileStream.seekp(0);
            m_FileStream.seekg(0);
        }
        else
        {
            m_FileStream = std::fstream(GetTempFileName(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
        }
    }

    void ZipAssetPack::DecompressCopy(
        const StringU8& FilePath)
    {
        OpenTempNew();

        bio::filtering_istream Filter;
        Filter.push(bio::gzip_decompressor());
        Filter.push(bio::file_source(FilePath, std::ios::in | std::ios::binary));

        m_FileStream << Filter.rdbuf();
    }

    void ZipAssetPack::CompressCopy(
        const StringU8& FilePath)
    {
        m_FileStream.seekp(0);
        m_FileStream.seekg(0);

        bio::filtering_ostream Filter;
        Filter.push(bio::gzip_compressor());
        Filter.push(bio::file_sink(FilePath, std::ios::out | std::ios::trunc | std::ios::binary));

        Filter << m_FileStream.rdbuf();
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

    size_t ZipAssetPack::SizeOfHeader(
        size_t NumberOfSections)
    {
        return sizeof(AssetPackHeader) + NumberOfSections * sizeof(AssetPackSection);
    }

    bool ZipAssetPack::ReadFile()
    {
        return Header_ReadSections() && Header_ReadBody();
    }

    bool ZipAssetPack::Header_ReadSections()
    {
        AssetPackHeader Header;
        m_FileStream.read(std::bit_cast<char*>(&Header), sizeof(Header));

        if (Header.Signature != AssetPackHeader::DefaultSignature ||
            Header.NumberOfResources == 0)
        {
            NEON_INFO("Resource", "Invalid header signature");
            return false;
        }

        AssetPackSection Section;
        for (uint16_t i = 0; i < Header.NumberOfResources; i++)
        {
            m_FileStream.read(std::bit_cast<char*>(&Section.Signature), sizeof(Section.Signature));
            if (Section.Signature != AssetPackSection::DefaultSignature)
            {
                NEON_INFO("Resource", "Invalid resource signature");
                return false;
            }

            m_FileStream.read(std::bit_cast<char*>(&Section.Handle), sizeof(Section.Handle));
            auto& Info = m_AssetsInfo[Section.Handle];
            if (Info.Size)
            {
                NEON_INFO("Resource", "Duplicate resource '{}'", buuid::to_string(Section.Handle));
                return false;
            }

            m_FileStream.read(std::bit_cast<char*>(&Info), sizeof(Info));
        }

        return true;
    }

    bool ZipAssetPack::Header_ReadBody()
    {
        bool Succeeded = true;

        // std::vector<std::future<void>> AssetsValidators;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            /*AssetsValidators.emplace_back(
                std::async(
                    [this, &Handle, &Info, &Succeeded]
                    {*/
            SHA256 Sha256;
            char   Byte;

            m_FileStream.seekg(Info.Offset);
            for (size_t i = 0; i < Info.Size && Succeeded; i++)
            {
                m_FileStream >> Byte;
                Sha256.Append(Byte);
            }

            if (Succeeded && Sha256.Digest() != Info.Hash)
            {
                NEON_INFO("Resource", "Invalid resource Sha256 for '{}'", buuid::to_string(Handle));
                Succeeded = false;
            }
            //}));
        }

        // AssetsValidators.clear();
        return Succeeded;
    }

    //

    void ZipAssetPack::WriteFile()
    {
        Header_WriteHeader();
        Header_WriteSections();
        Header_WriteBody();
    }

    void ZipAssetPack::Header_WriteHeader()
    {
        m_FileStream.seekp(0);

        AssetPackHeader Header{
            .Signature         = AssetPackHeader::DefaultSignature,
            .Version           = AssetPackHeader::LatestVersion,
            .NumberOfResources = uint16_t(m_AssetsInfo.size())
        };
        m_FileStream.write(std::bit_cast<const char*>(&Header), sizeof(Header));
    }

    void ZipAssetPack::Header_WriteSections()
    {
        m_FileStream.seekp(sizeof(AssetPackHeader));

        auto Signature = AssetPackSection::DefaultSignature;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            m_FileStream.write(std::bit_cast<const char*>(&Signature), sizeof(Signature));
            m_FileStream.write(std::bit_cast<const char*>(&Handle), sizeof(Handle));
            m_FileStream.write(std::bit_cast<const char*>(&Info), sizeof(Info));
        }
    }

    void ZipAssetPack::Header_WriteBody()
    {
        m_FileStream.seekp(SizeOfHeader(m_AssetsInfo.size()));

        std::vector<std::future<void>> AssetsWriter;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            // AssetsWriter.emplace_back(
            // std::async(
            //[this, &Handle, &Info]
            //{

            auto  Handler  = m_Handlers.Get(Info.LoaderId);
            auto& Resource = m_LoadedAssets[Handle];

            auto DataPos = m_FileStream.tellp();
            Handler->Save(Resource, m_FileStream, Info.Size);
            auto RestorePos = m_FileStream.tellp();

            m_FileStream.seekp(DataPos);
            SHA256 Sha256;
            Sha256.Append(m_FileStream, RestorePos - DataPos);
            Info.Hash = Sha256.Digest();
            //}));
        }
    }
} // namespace Neon::Asset
