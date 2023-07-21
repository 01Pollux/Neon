#include <ResourcePCH.hpp>
#include <Asset/Metadata.hpp>

namespace Neon::AAsset
{
    AssetMetaDataDef::AssetMetaDataDef(
        AssetMetaData MetaData) :
        m_MetaData(std::move(MetaData))
    {
        auto Iter = m_MetaData.find("LoaderData");
        if (Iter != m_MetaData.not_found())
        {
            m_LoaderData = &Iter->second;
        }
        else
        {
            m_LoaderData = &m_MetaData.add_child("LoaderData", boost::property_tree::ptree());
        }
    }

    AssetMetaDataDef::AssetMetaDataDef()
    {
        m_LoaderData = &m_MetaData.add_child("LoaderData", boost::property_tree::ptree());
    }

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
} // namespace Neon::AAsset