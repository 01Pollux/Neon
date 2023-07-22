#include <ResourcePCH.hpp>
#include <Asset/Metadata.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace Neon::AAsset
{
    AssetMetaDataDef::AssetMetaDataDef(
        std::ifstream& Stream)
    {
        boost::property_tree::read_ini(Stream, m_MetaData);
        auto Iter = m_MetaData.find("Loader");
        if (Iter != m_MetaData.not_found())
        {
            m_LoaderData = &Iter->second;
        }
        else
        {
            m_LoaderData = &m_MetaData.add_child("Loader", boost::property_tree::ptree());
        }
    }

    AssetMetaDataDef::AssetMetaDataDef(
        const Handle& AssetGuid,
        StringU8      Path) :
        m_Path(std::move(Path)),
        m_IsDirty(true)
    {
        SetGuid(AssetGuid);
        m_LoaderData = &m_MetaData.add_child("Loader", boost::property_tree::ptree());
    }

    void AssetMetaDataDef::Export(
        std::ofstream& Stream)
    {
        boost::property_tree::write_ini(Stream, m_MetaData);
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

    AssetMetaData& AssetMetaDataDef::GetLoaderData() noexcept
    {
        return *m_LoaderData;
    }

    const AssetMetaData& AssetMetaDataDef::GetLoaderData() const noexcept
    {
        return *m_LoaderData;
    }

    const StringU8& AssetMetaDataDef::GetPath() const noexcept
    {
        return m_Path;
    }

    void AssetMetaDataDef::SetPath(
        StringU8 Path) noexcept
    {
        m_Path = std::move(Path);
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
} // namespace Neon::AAsset