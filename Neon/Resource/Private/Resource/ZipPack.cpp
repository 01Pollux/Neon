#include <ResourcePCH.hpp>
#include <Resource/Packs/ZipPack.hpp>
#include <Resource/Handler.hpp>

#include <future>
#include <filesystem>

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
        m_FileStream = std::fstream(GetTempFileName(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
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
        std::scoped_lock Lock(m_AsyncMutex, m_PackMutex);

        m_AssetsInfo.clear();
        m_LoadedAssets.clear();

        DecompressCopy(FilePath);

        try
        {
            if (!ReadFile())
            {
                m_FileStream = {};
                NEON_ERROR_TAG("Resource", "Invalid pack file '{}'", FilePath);
            }
        }
        catch (const std::exception& Exception)
        {
            NEON_ERROR_TAG("Resource", "Exception caught while trying to read file '{}', ({})", FilePath, Exception.what());
        }
    }

    void ZipAssetPack::Export(
        const StringU8& FilePath)
    {
        std::scoped_lock Async(m_AsyncMutex, m_PackMutex);

        size_t FileSize = OffsetToBody();
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            if (!m_Handlers.Get(Info.LoaderId))
            {
                NEON_WARNING_TAG("Resource", "Tried to export a resource '{}' with an unknown handler", buuid::to_string(Handle));
                continue;
            }

            Info.Offset = FileSize;
            FileSize += Info.Size;
        }

        ResetReadWrite();
        WriteFile();

        CompressCopy(FilePath);
    }

    //

    Ref<IAssetResource> ZipAssetPack::Load(
        const AssetHandle& Handle)
    {
        std::scoped_lock Lock(m_AsyncMutex, m_PackMutex);

        auto& LoadedAsset = m_LoadedAssets[Handle];

        // Resource already loaded
        if (LoadedAsset)
        {
            return LoadedAsset;
        }

        NEON_INFO_TAG("Resource", "Loading asset: {}", buuid::to_string(Handle));

        StringU8 ErrorText;
        auto     Asset = LoadAsset(m_Handlers, Handle, ErrorText);

        if (!ErrorText.empty())
        {
            NEON_WARNING_TAG("Resource", ErrorText, buuid::to_string(Handle));
        }
        else
        {
            LoadedAsset = Asset;
        }

        return Asset;
    }

    void ZipAssetPack::Save(
        const AssetHandle&         Handle,
        const Ptr<IAssetResource>& Resource)
    {
        std::scoped_lock Lock(m_AsyncMutex, m_PackMutex);

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

        NEON_WARNING_TAG("Resource", "No handler support resource '{}'", buuid::to_string(Handle));
    }

    auto ZipAssetPack::ContainsResource(
        const AssetHandle& Handle) const -> ContainType
    {
        std::scoped_lock Lock(m_AsyncMutex, m_PackMutex);
        if (m_LoadedAssets.contains(Handle))
        {
            return ContainType::Loaded;
        }
        return m_AssetsInfo.contains(Handle) ? ContainType::NotLoaded : ContainType::Missing;
    }

    //

    auto ZipAssetPack::GetAssets() const -> AssetHandleList
    {
        std::scoped_lock Lock(m_AsyncMutex, m_PackMutex);
        AssetHandleList  Assets;
        Assets.insert_range(Assets.begin(), m_AssetsInfo | std::views::keys);
        return Assets;
    }

    Ptr<IAssetResource> ZipAssetPack::LoadAsset(
        const AssetResourceHandlers& Handlers,
        const AssetHandle&           Handle,
        StringU8&                    ErrorText)
    {
        auto Iter = m_AssetsInfo.find(Handle);
        if (Iter == m_AssetsInfo.end())
        {
            ErrorText = "Tried loading asset '{}' that doesn't exists";
            return nullptr;
        }

        auto& Info = Iter->second;
        m_FileStream.seekg(Info.Offset);

        auto Handler = Handlers.Get(Info.LoaderId);
        if (!Handler)
        {
            ErrorText = "Tried loading asset '{}' with handler that doesn't exists";
            return nullptr;
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

    void ZipAssetPack::ResetReadWrite()
    {
        m_FileStream.seekp(0);
        m_FileStream.seekg(0);
    }

    void ZipAssetPack::DecompressCopy(
        const StringU8& FilePath)
    {
        ResetReadWrite();

        bio::filtering_istream Filter;
        Filter.push(bio::gzip_decompressor());
        Filter.push(bio::file_source(FilePath, std::ios::in | std::ios::binary));

        m_FileStream << Filter.rdbuf();
    }

    void ZipAssetPack::CompressCopy(
        const StringU8& FilePath)
    {
        ResetReadWrite();

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

        uint16_t      Signature = DefaultSignature;
        uint16_t      NumberOfResources;
        SHA256::Bytes Hash;
        uint8_t       Version = LatestVersion;
    };

    struct AssetPackSection
    {
        static constexpr uint16_t DefaultSignature = AssetPackHeader::DefaultSignature;

        uint16_t               Signature = DefaultSignature;
        AssetHandle            Handle;
        ZipAssetPack::PackInfo PackInfo;
    };

    //

    size_t ZipAssetPack::OffsetToBody() const
    {
        return sizeof(AssetPackHeader) + m_AssetsInfo.size() * sizeof(AssetPackSection);
    }

    //

    bool ZipAssetPack::ReadFile()
    {
        SHA256 Sha256;

        AssetPackHeader HeaderInfo;
        if (!Header_ReadHeader(HeaderInfo) || !Header_ReadSections(Sha256, HeaderInfo))
        {
            return false;
        }

        Sha256.Append(AssetPackHeader::DefaultSignature);
        Sha256.Append(m_AssetsInfo.size());

        NEON_VALIDATE(Sha256.Digest() == HeaderInfo.Hash, "Invalid pack checksum");

        Header_ReadBody();
        return true;
    }

    bool ZipAssetPack::Header_ReadHeader(
        AssetPackHeader& HeaderInfo)
    {
        m_FileStream.seekg(0);

        m_FileStream.read(std::bit_cast<char*>(&HeaderInfo), sizeof(HeaderInfo));
        NEON_ASSERT(m_FileStream.tellg() == sizeof(AssetPackHeader));

        if (HeaderInfo.Signature != AssetPackHeader::DefaultSignature ||
            HeaderInfo.NumberOfResources == 0)
        {
            NEON_INFO_TAG("Resource", "Invalid header's information");
            return false;
        }

        NEON_ASSERT(m_FileStream.tellg() == sizeof(AssetPackHeader));
        return true;
    }

    bool ZipAssetPack::Header_ReadSections(
        SHA256&                Header,
        const AssetPackHeader& HeaderInfo)
    {
        m_FileStream.seekg(sizeof(AssetPackHeader));

        AssetPackSection Section;
        for (uint16_t i = 0; i < HeaderInfo.NumberOfResources; i++)
        {
            m_FileStream.read(std::bit_cast<char*>(&Section), sizeof(Section));
            if (Section.Signature != AssetPackSection::DefaultSignature ||
                !Section.PackInfo.Size)
            {
                NEON_INFO_TAG("Resource", "Invalid resource signature");
                return false;
            }

            auto [InfoIter, Inserted] = m_AssetsInfo.emplace(Section.Handle, Section.PackInfo);
            if (!Inserted)
            {
                NEON_INFO_TAG("Resource", "Duplicate resource '{}'", buuid::to_string(Section.Handle));
                return false;
            }

            Header.Append(std::bit_cast<const uint8_t*>(&Section.PackInfo), sizeof(Section.PackInfo));
        }

        NEON_ASSERT(m_FileStream.tellg() == OffsetToBody());
        return true;
    }

    void ZipAssetPack::Header_ReadBody()
    {
        m_FileStream.seekg(OffsetToBody());

        SHA256 Sha256;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            m_FileStream.seekg(Info.Offset);

            Sha256.Reset();
            Sha256.Append(m_FileStream, Info.Size);

            auto Hash = Sha256.Digest();
            NEON_VALIDATE(Hash == Info.Hash, "Invalid resource checksum for '{}'", buuid::to_string(Handle));
        }
    }

    //

    void ZipAssetPack::WriteFile()
    {
        SHA256 Header;
        Header_WriteBody();
        Header_WriteSections(Header);
        Header_WriteHeader(Header);
    }

    void ZipAssetPack::Header_WriteHeader(
        SHA256& Header)
    {
        m_FileStream.seekp(0);

        Header.Append(AssetPackHeader::DefaultSignature);
        Header.Append(m_AssetsInfo.size());

        AssetPackHeader HeaderInfo{
            .NumberOfResources = uint16_t(m_AssetsInfo.size()),
            .Hash              = Header.Digest(),
        };

        m_FileStream.write(std::bit_cast<const char*>(&HeaderInfo), sizeof(HeaderInfo));
        NEON_ASSERT(m_FileStream.tellp() == sizeof(AssetPackHeader));
    }

    void ZipAssetPack::Header_WriteSections(
        SHA256& Header)
    {
        m_FileStream.seekp(sizeof(AssetPackHeader));

        AssetPackSection Section;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            Section.PackInfo = Info;
            Section.Handle   = Handle;
            m_FileStream.write(std::bit_cast<const char*>(&Section), sizeof(Section));
            Header.Append(std::bit_cast<const uint8_t*>(&Info), sizeof(Info));
        }

        NEON_ASSERT(m_FileStream.tellp() == OffsetToBody());
    }

    void ZipAssetPack::Header_WriteBody()
    {
        m_FileStream.seekp(OffsetToBody());

        SHA256 Sha256;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            auto  Handler  = m_Handlers.Get(Info.LoaderId);
            auto& Resource = m_LoadedAssets[Handle];

            auto DataPos = m_FileStream.tellp();
            Handler->Save(Resource, m_FileStream, Info.Size);

            auto RestorePos = m_FileStream.tellp();
            m_FileStream.seekp(DataPos);

            // Process the hash
            Sha256.Reset();
            Sha256.Append(m_FileStream, RestorePos - DataPos);
            Info.Hash = Sha256.Digest();
        }
    }
} // namespace Neon::Asset
