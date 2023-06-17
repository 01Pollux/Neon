#include <ResourcePCH.hpp>
#include <Resource/Packs/ZipPack.hpp>
#include <Resource/Handler.hpp>
#include <Resource/Operator.hpp>
#include <IO/Archive.hpp>

#include <future>
#include <filesystem>
#include <ranges>

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
        m_Dependencies.clear();

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
        IterateDepencies(
            Handle,
            [this](const AssetHandle& Dependency)
            {
                Load(Dependency);
            });

        auto Lock = m_DefferedOperator.Lock(this, m_PackMutex);

        auto& LoadedAsset = m_LoadedAssets[Handle];

        // Resource already loaded
        if (!LoadedAsset)
        {
            NEON_TRACE_TAG("Resource", "Loading asset: {}", Handle.ToString());

            if (auto Asset = LoadAsset(m_Handlers, Handle))
            {
                LoadedAsset = *Asset;
            }
            else
            {
                NEON_WARNING_TAG("Resource", Asset.error(), Handle.ToString());
            }
        }

        return LoadedAsset;
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

    std::expected<Ptr<IAssetResource>, const char*> ZipAssetPack::LoadAsset(
        const AssetResourceHandlers& Handlers,
        const AssetHandle&           Handle)
    {
        auto Iter = m_AssetsInfo.find(Handle);
        if (Iter == m_AssetsInfo.end())
        {
            return std::unexpected("Tried loading asset '{}' that doesn't exists");
        }

        auto& Info = Iter->second;
        m_File.seekg(Info.Offset);

        auto Handler = Handlers.Get(Info.LoaderId);
        if (!Handler)
        {
            return std::unexpected("Tried loading asset '{}' with handler that doesn't exists");
        }

        IO::InArchive Archive(m_File);
        return Handler->Load(this, Archive, Info.Size);
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

        // create directory of the FilePath if it doesnt exists
        std::filesystem::path Path(FilePath);
        if (auto ParentPath = Path.parent_path(); !ParentPath.empty())
        {
            if (!std::filesystem::exists(ParentPath))
                std::filesystem::create_directories(ParentPath);
        }

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

        struct DependencyList
        {
            static size_t GetSize(
                const IAssetPack::AssetDependencyMap& DependencyMap)
            {
                size_t Size = sizeof(uint16_t);
                for (auto& [Resource, Sets] : DependencyMap)
                {
                    Size += sizeof(uint16_t);
                    Size += sizeof(AssetHandle);
                    Size += Resource.size() * sizeof(AssetHandle);
                }
                return Size;
            }

            static void Read(
                IO::BinaryStreamReader          Stream,
                IAssetPack::AssetDependencyMap& DependencyMap)
            {
                auto NumberOfDependencies = Stream.Read<uint16_t>();
                for (uint16_t i = 0; i < NumberOfDependencies; ++i)
                {
                    auto& Resource = DependencyMap[Stream.Read<AssetHandle>()];
                    auto  Size     = Stream.Read<uint16_t>();
                    for (uint16_t j = 0; j < Size; ++j)
                    {
                        Resource.insert(Stream.Read<AssetHandle>());
                    }
                }
            }

            static void Write(
                IO::BinaryStreamWriter                Stream,
                const IAssetPack::AssetDependencyMap& DependencyMap)
            {
                Stream.Write(uint16_t(DependencyMap.size()));
                for (auto& [Resource, Sets] : DependencyMap)
                {
                    Stream.Write(Resource);
                    Stream.Write(uint16_t(Sets.size()));
                    for (auto& Set : Sets)
                    {
                        Stream.Write(Set);
                    }
                }
            }
        };

        void Read(
            IO::BinaryStreamReader          Stream,
            IAssetPack::AssetDependencyMap& DependencyMap)
        {
            Stream.Read(Signature);
            Stream.Read(NumberOfResources);
            Stream.ReadBytes(Hash.data(), Hash.size());
            Stream.Read(Version);
            DependencyList::Read(Stream, DependencyMap);
        }

        void Write(
            IO::BinaryStreamWriter          Stream,
            IAssetPack::AssetDependencyMap& DependencyMap) const
        {
            Stream.Write(Signature);
            Stream.Write(NumberOfResources);
            Stream.WriteBytes(Hash.data(), Hash.size());
            Stream.Write(Version);
            DependencyList::Write(Stream, DependencyMap);
        }

    public:
        static constexpr size_t GetOffset() noexcept
        {
            return 0;
        }

        static size_t GetSize(
            const IAssetPack::AssetDependencyMap& DependencyMap) noexcept
        {
            return sizeof(AssetPackHeader) + DependencyList::GetSize(DependencyMap);
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

    public:
        static size_t GetOffset(
            const IAssetPack::AssetDependencyMap& DependencyMap) noexcept
        {
            return AssetPackHeader::GetSize(DependencyMap);
        }

        static constexpr size_t GetSize(
            size_t NumberOfAssets) noexcept
        {
            return NumberOfAssets * sizeof(AssetPackSection);
        }
    };

    size_t ZipAssetPack::OffsetToBody() const
    {
        return AssetPackHeader::GetSize(m_Dependencies) + AssetPackSection::GetSize(m_AssetsInfo.size());
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
        m_File.seekg(AssetPackHeader::GetOffset());
        HeaderInfo.Read(m_File, m_Dependencies);

        NEON_TRACE_TAG("Resource", "Reading header");
        NEON_TRACE_TAG("Resource", "Header Version: {}", HeaderInfo.Version);
        NEON_TRACE_TAG("Resource", "Number of resources: {}", HeaderInfo.NumberOfResources);
        NEON_TRACE_TAG("Resource", "Number of dependencyMap: {}", m_Dependencies.size());
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
        m_File.seekg(AssetPackSection::GetOffset(m_Dependencies));
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
        auto         FilePath = GetTempFileName() + "_tmp";
        std::fstream FinalFile(FilePath, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);

        SHA256 Header;
        Header_WriteBody(FinalFile);
        Header_WriteSections(FinalFile, Header);
        Header_WriteHeader(FinalFile, Header);

        NEON_TRACE_TAG("Resource", "------------------------------------------------------");

        m_File.seekp(0);
        FinalFile.seekg(0);
        m_File << FinalFile.rdbuf();
        FinalFile.close();

        std::filesystem::remove(FilePath);
    }

    void ZipAssetPack::Header_WriteHeader(
        std::fstream& FinalFile,
        SHA256&       Header)
    {
        FinalFile.seekp(AssetPackHeader::GetOffset());
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

        HeaderInfo.Write(FinalFile, m_Dependencies);
    }

    void ZipAssetPack::Header_WriteSections(
        std::fstream& FinalFile,
        SHA256&       Header)
    {
        FinalFile.seekp(AssetPackSection::GetOffset(m_Dependencies));
        AssetPackSection Section;

        NEON_TRACE_TAG("Resource", "Writing Sections");
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            Section.PackInfo = Info;
            Section.Handle   = Handle;
            Section.Write(FinalFile);
            Header.Append(std::bit_cast<const uint8_t*>(&Info), sizeof(Info));

            NEON_TRACE_TAG("Resource", "Writing Section: {}", Section.Handle.ToString());
            NEON_TRACE_TAG("Resource", "Signature: {:X}", Section.Signature);
            NEON_TRACE_TAG("Resource", "Loader: {:X}", Section.PackInfo.LoaderId);
            NEON_TRACE_TAG("Resource", "Offset: {}", Section.PackInfo.Offset);
            NEON_TRACE_TAG("Resource", "Size: {}", Section.PackInfo.Size);
        }
    }

    void ZipAssetPack::Header_WriteBody(
        std::fstream& FinalFile)
    {
        FinalFile.seekp(OffsetToBody());
        SHA256 Sha256;
        for (auto& [Handle, Info] : m_AssetsInfo)
        {
            auto  Handler  = m_Handlers.Get(Info.LoaderId);
            auto& Resource = m_LoadedAssets[Handle];

            // If the data was loaded, write it to the file
            if (Resource)
            {
                auto DataPos = FinalFile.tellp();
                Info.Offset  = DataPos;

                IO::OutArchive Archive(FinalFile);
                Handler->Save(this, Resource, Archive);

                Info.Size = FinalFile.tellp() - DataPos;
                FinalFile.seekp(DataPos);

                // Process the hash
                Sha256.Reset();
                Sha256.Append(FinalFile, Info.Size);
                Info.Hash = Sha256.Digest();
            }
            else
            {
                m_File.seekg(Info.Offset);

                IO::BinaryStreamWriter Stream(FinalFile);
                IO::BinaryStreamReader Reader(m_File);

                std::copy_n(
                    std::istreambuf_iterator(Reader.Get()),
                    Info.Size,
                    std::ostreambuf_iterator<char>(Stream.Get()));
            }
        }
    }
} // namespace Neon::Asset
