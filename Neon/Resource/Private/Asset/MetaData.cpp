#include <ResourcePCH.hpp>
#include <Asset/Metadata.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Log/Logger.hpp>

namespace Neon::Asset
{
    AssetMetaDataDef::AssetMetaDataDef(
        std::ifstream& Stream)
    {
        boost::property_tree::read_json(Stream, m_MetaData);
        if (m_MetaData.find("Loader") == m_MetaData.not_found())
        {
            m_MetaData.put_child("Loader", boost::property_tree::ptree());
        }
        if (m_MetaData.find("Dependencies") == m_MetaData.not_found())
        {
            m_MetaData.put_child("Dependencies", boost::property_tree::ptree());
        }
    }

    AssetMetaDataDef::AssetMetaDataDef(
        const Handle& AssetGuid,
        StringU8      Path) :
        m_IsDirty(true)
    {
        SetGuid(AssetGuid);
        SetPath(std::move(Path));
        m_MetaData.add_child("Loader", boost::property_tree::ptree());
        m_MetaData.add_child("Dependencies", boost::property_tree::ptree());
    }

    void AssetMetaDataDef::Export(
        std::ofstream& Stream)
    {
        boost::property_tree::write_json(Stream, m_MetaData);
    }

    //

    Handle AssetMetaDataDef::GetGuid() const noexcept
    {
        auto Iter = m_MetaData.find("Guid");
        return Iter != m_MetaData.not_found() ? Handle::FromString(Iter->second.get_value<std::string>()) : Handle::Null;
    }

    void AssetMetaDataDef::SetGuid(
        const Handle& Guid) noexcept
    {
        m_MetaData.put("Guid", Guid.ToString());
    }

    StringU8 AssetMetaDataDef::GetHash() const noexcept
    {
        auto Iter = m_MetaData.find("Hash");
        return Iter != m_MetaData.not_found() ? Iter->second.get_value<std::string>() : StringU8();
    }

    void AssetMetaDataDef::SetHash(
        StringU8 Hash) noexcept
    {
        m_MetaData.put("Hash", std::move(Hash));
    }

    size_t AssetMetaDataDef::GetLoaderId() const noexcept
    {
        return m_MetaData.get_optional<size_t>("LoaderId").value_or(0);
    }

    void AssetMetaDataDef::SetLoaderId(
        size_t Id) noexcept
    {
        m_MetaData.put("LoaderId", Id);
    }

    AssetMetaData& AssetMetaDataDef::GetLoaderData() noexcept
    {
        return m_MetaData.get_child("Loader");
    }

    const AssetMetaData& AssetMetaDataDef::GetLoaderData() const noexcept
    {
        return m_MetaData.get_child("Loader");
    }

    std::filesystem::path AssetMetaDataDef::GetAssetPath() const
    {
        return FileSystem::ConvertToUnixPath(std::filesystem::path(GetPath()).replace_extension(""));
    }

    StringU8 AssetMetaDataDef::GetPath() const
    {
        auto Path = m_MetaData.get<StringU8>("Path");
        NEON_VALIDATE(!(Path.empty() || Path.starts_with("..")), "Path '{}' cannot be empty or start with '..'", Path);
        return Path;
    }

    void AssetMetaDataDef::SetPath(
        StringU8 Path)
    {
        NEON_VALIDATE(!(Path.empty() || Path.starts_with("..")), "Path '{}' cannot be empty or start with '..'", Path);
        m_MetaData.put("Path", std::move(Path));
    }

    bool AssetMetaDataDef::IsDirty() const noexcept
    {
        return m_IsDirty;
    }

    void AssetMetaDataDef::SetDirty(
        bool IsDirty) noexcept
    {
        m_IsDirty = IsDirty;
    }

    Asio::CoGenerator<Handle> AssetMetaDataDef::GetDependencies() const
    {
        for (auto& Dependency : m_MetaData.get_child("Dependencies"))
        {
            co_yield Handle::FromString(Dependency.second.get_value<StringU8>());
        }
    }

    void AssetMetaDataDef::SetDependencies(
        std::list<StringU8> Dependencies)
    {
        auto& DepsNode = m_MetaData.get_child("Dependencies");
        DepsNode.clear();
        for (auto& Dependency : Dependencies)
        {
            DepsNode.push_back({ "", boost::property_tree::ptree(std::move(Dependency)) });
        }
    }
} // namespace Neon::Asset