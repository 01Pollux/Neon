#include <ResourcePCH.hpp>
#include <Resource/Packs/ZipPack.hpp>
#include <Resource/Handler.hpp>
#include <Resource/Operator.hpp>

#include <future>
#include <filesystem>

#include <boost/pool/pool.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <Log/Logger.hpp>

namespace bio = boost::iostreams;

namespace Neon::Asset
{
    ZipAssetPack::ZipAssetPack(
        const AssetResourceHandlers& Handlers,
        DeferredResourceOperator&    DefferedOperator) :
        IAssetPack(Handlers, DefferedOperator),
        m_File(GetTempFileName(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary)
    {
    }

    ZipAssetPack::~ZipAssetPack()
    {
        if (m_File)
        {
            m_File = {};
            std::filesystem::remove(GetTempFileName());
        }
    }

    void ZipAssetPack::Import(
        const StringU8& FilePath)
    {
        auto Lock = m_DefferedOperator.Lock(this, m_PackMutex);

        m_AssetsInfo.clear();
        m_LoadedAssets.clear();

        try
        {
            DecompressCopy(FilePath);
            if (!ReadFile())
            {
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
        auto Lock = m_DefferedOperator.Lock(this, m_PackMutex);

        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            if (!m_Handlers.Get(Info.LoaderId))
            {
                NEON_WARNING_TAG("Resource", "Tried to export a resource '{}' with an unknown handler", Handle.ToString());
                continue;
            }
        }

        ResetReadWrite();
        WriteFile();

        CompressCopy(FilePath);
    }

    //

    Ref<IAssetResource> ZipAssetPack::Load(
        const AssetHandle& Handle)
    {
        auto Lock = m_DefferedOperator.Lock(this, m_PackMutex);

        auto& LoadedAsset = m_LoadedAssets[Handle];

        // Resource already loaded
        if (LoadedAsset)
        {
            return LoadedAsset;
        }

        NEON_TRACE_TAG("Resource", "Loading asset: {}", Handle.ToString());

        StringU8 ErrorText;
        auto     Asset = LoadAsset(m_Handlers, Handle, ErrorText);

        if (!ErrorText.empty())
        {
            NEON_WARNING_TAG("Resource", ErrorText, Handle.ToString());
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
        auto Lock = m_DefferedOperator.Lock(this, m_PackMutex);

        for (auto& [LoaderId, Handler] : m_Handlers.Get())
        {
            if (Handler->CanCastTo(Resource))
            {
                if (std::exchange(m_LoadedAssets[Handle], Resource) != Resource)
                {
                    auto& Info    = m_AssetsInfo[Handle];
                    Info.LoaderId = LoaderId;
                }
                return;
            }
        }

        NEON_WARNING_TAG("Resource", "No handler support resource '{}'", Handle.ToString());
    }

    auto ZipAssetPack::ContainsResource(
        const AssetHandle& Handle) const -> ContainType
    {
        auto Lock = m_DefferedOperator.Lock(this, m_PackMutex);
        if (m_LoadedAssets.contains(Handle))
        {
            return ContainType::Loaded;
        }
        return m_AssetsInfo.contains(Handle) ? ContainType::NotLoaded : ContainType::Missing;
    }

    //

    auto ZipAssetPack::GetAssets() const -> AssetHandleList
    {
        auto Lock = m_DefferedOperator.Lock(this, m_PackMutex);

        AssetHandleList Assets;
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
        m_File.seekg(Info.Offset);

        auto Handler = Handlers.Get(Info.LoaderId);
        if (!Handler)
        {
            ErrorText = "Tried loading asset '{}' with handler that doesn't exists";
            return nullptr;
        }

        return Handler->Load(m_File, Info.Size);
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
        m_File.seekp(0);
        m_File.seekg(0);
    }

    void ZipAssetPack::DecompressCopy(
        const StringU8& FilePath)
    {
        bio::file_source File(FilePath, std::ios::in | std::ios::binary);
        if (!File.is_open())
        {
            throw std::runtime_error(StringUtils::Format("Failed to open file '{}'", FilePath));
        }

        ResetReadWrite();

        bio::filtering_istream Filter;
        Filter.push(bio::gzip_decompressor());
        Filter.push(File);
        if (Filter.bad())
        {
            throw std::runtime_error(StringUtils::Format("Failed to decompress file '{}'", FilePath));
        }
        Filter.seekg(0);
        m_File << Filter.rdbuf();
    }

    void ZipAssetPack::CompressCopy(
        const StringU8& FilePath)
    {
        ResetReadWrite();

        bio::filtering_ostream Filter;
        Filter.push(bio::gzip_compressor());
        Filter.push(bio::file_sink(FilePath, std::ios::out | std::ios::trunc | std::ios::binary));

        Filter << m_File.rdbuf();
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

        void Read(
            IO::BinaryStreamReader Stream)
        {
            Stream.Read(Signature);
            Stream.Read(NumberOfResources);
            Stream.ReadBytes(Hash.data(), Hash.size());
            Stream.Read(Version);
        }

        void Write(
            IO::BinaryStreamWriter Stream) const
        {
            Stream.Write(Signature);
            Stream.Write(NumberOfResources);
            Stream.WriteBytes(Hash.data(), Hash.size());
            Stream.Write(Version);
        }
    };

    struct AssetPackSection
    {
        static constexpr uint16_t DefaultSignature = AssetPackHeader::DefaultSignature;

        uint16_t               Signature = DefaultSignature;
        AssetHandle            Handle;
        ZipAssetPack::PackInfo PackInfo;

        void Read(
            IO::BinaryStreamReader Stream)
        {
            Stream.Read(Signature);
            Stream.ReadBytes(Handle.data, Handle.size());
            Stream.ReadBytes(PackInfo.Hash.data(), PackInfo.Hash.size());
            Stream.Read(PackInfo.LoaderId);
            Stream.Read(PackInfo.Offset);
            Stream.Read(PackInfo.Size);
        }

        void Write(
            IO::BinaryStreamWriter Stream) const
        {
            Stream.Write(Signature);
            Stream.WriteBytes(Handle.data, Handle.size());
            Stream.WriteBytes(PackInfo.Hash.data(), PackInfo.Hash.size());
            Stream.Write(PackInfo.LoaderId);
            Stream.Write(PackInfo.Offset);
            Stream.Write(PackInfo.Size);
        }
    };

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
        NEON_TRACE_TAG("Resource", "------------------------------------------------------");
        return true;
    }

    bool ZipAssetPack::Header_ReadHeader(
        AssetPackHeader& HeaderInfo)
    {
        m_File.seekg(0);
        HeaderInfo.Read(m_File);

        NEON_TRACE_TAG("Resource", "Reading header");
        NEON_TRACE_TAG("Resource", "Header Version: {}", HeaderInfo.Version);
        NEON_TRACE_TAG("Resource", "Number of resources: {}", HeaderInfo.NumberOfResources);
        NEON_TRACE_TAG("Resource", "Signature: {:X}", HeaderInfo.Signature);

        if (HeaderInfo.Signature != AssetPackHeader::DefaultSignature ||
            HeaderInfo.NumberOfResources == 0)
        {
            NEON_TRACE_TAG("Resource", "Invalid header's information");
            return false;
        }

        return true;
    }

    bool ZipAssetPack::Header_ReadSections(
        SHA256&                Header,
        const AssetPackHeader& HeaderInfo)
    {
        m_File.seekg(sizeof(AssetPackHeader));
        AssetPackSection Section;

        NEON_TRACE_TAG("Resource", "Reading sections");
        for (uint16_t i = 0; i < HeaderInfo.NumberOfResources; i++)
        {
            Section.Read(m_File);

            NEON_TRACE_TAG("Resource", "Section: {}", Section.Handle.ToString());
            NEON_TRACE_TAG("Resource", "Signature: {:X}", Section.Signature);
            NEON_TRACE_TAG("Resource", "Loader: {:X}", Section.PackInfo.LoaderId);
            NEON_TRACE_TAG("Resource", "Offset: {}", Section.PackInfo.Offset);
            NEON_TRACE_TAG("Resource", "Size: {}", Section.PackInfo.Size);

            if (Section.Signature != AssetPackSection::DefaultSignature)
            {
                NEON_WARNING_TAG("Resource", "Invalid resource signature");
                return false;
            }

            auto [InfoIter, Inserted] = m_AssetsInfo.emplace(Section.Handle, Section.PackInfo);
            if (!Inserted)
            {
                NEON_WARNING_TAG("Resource", "Duplicate resource '{}'", Section.Handle.ToString());
                return false;
            }

            Header.Append(std::bit_cast<const uint8_t*>(&Section.PackInfo), sizeof(Section.PackInfo));
        }

        return true;
    }

    void ZipAssetPack::Header_ReadBody()
    {
        m_File.seekg(OffsetToBody());
        SHA256 Sha256;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            m_File.seekg(Info.Offset);

            Sha256.Reset();
            Sha256.Append(m_File, Info.Size);

            auto Hash = Sha256.Digest();
            NEON_VALIDATE(Hash == Info.Hash, "Invalid resource checksum for '{}'", Handle.ToString());
        }
    }

    //

    void ZipAssetPack::WriteFile()
    {
        SHA256 Header;
        Header_WriteBody();
        Header_WriteSections(Header);
        Header_WriteHeader(Header);
        NEON_TRACE_TAG("Resource", "------------------------------------------------------");
    }

    void ZipAssetPack::Header_WriteHeader(
        SHA256& Header)
    {
        m_File.seekp(0);
        Header.Append(AssetPackHeader::DefaultSignature);
        Header.Append(m_AssetsInfo.size());

        AssetPackHeader HeaderInfo{
            .NumberOfResources = uint16_t(m_AssetsInfo.size()),
            .Hash              = Header.Digest(),
        };

        NEON_TRACE_TAG("Resource", "Writing Header");
        NEON_TRACE_TAG("Resource", "Header Version: {}", HeaderInfo.Version);
        NEON_TRACE_TAG("Resource", "Number of resources: {}", HeaderInfo.NumberOfResources);
        NEON_TRACE_TAG("Resource", "Signature: {:X}", HeaderInfo.Signature);

        HeaderInfo.Write(m_File);
    }

    void ZipAssetPack::Header_WriteSections(
        SHA256& Header)
    {
        m_File.seekp(sizeof(AssetPackHeader));
        AssetPackSection Section;

        NEON_TRACE_TAG("Resource", "Writing Sections");
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            Section.PackInfo = Info;
            Section.Handle   = Handle;
            Section.Write(m_File);
            Header.Append(std::bit_cast<const uint8_t*>(&Info), sizeof(Info));

            NEON_TRACE_TAG("Resource", "Writing Section: {}", Section.Handle.ToString());
            NEON_TRACE_TAG("Resource", "Signature: {:X}", Section.Signature);
            NEON_TRACE_TAG("Resource", "Loader: {:X}", Section.PackInfo.LoaderId);
            NEON_TRACE_TAG("Resource", "Offset: {}", Section.PackInfo.Offset);
            NEON_TRACE_TAG("Resource", "Size: {}", Section.PackInfo.Size);
        }
    }

    void ZipAssetPack::Header_WriteBody()
    {
        m_File.seekp(OffsetToBody());
        SHA256 Sha256;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            auto  Handler  = m_Handlers.Get(Info.LoaderId);
            auto& Resource = m_LoadedAssets[Handle];

            auto DataPos = m_File.tellp();
            Info.Offset  = DataPos;
            Handler->Save(Resource, m_File);

            Info.Size = m_File.tellp() - DataPos;
            m_File.seekp(DataPos);

            // Process the hash
            Sha256.Reset();
            Sha256.Append(m_File, Info.Size);
            Info.Hash = Sha256.Digest();
        }
    }
} // namespace Neon::Asset
