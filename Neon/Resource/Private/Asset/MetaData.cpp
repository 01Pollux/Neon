#include <ResourcePCH.hpp>
#include <Asset/Metadata.hpp>

namespace Neon::AAsset
{
    AssetMetaData::AssetMetaData(
        boost::property_tree::ptree& MetaData) :
        m_MetaData(MetaData)
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

    Handle AssetMetaData::GetGuid() const noexcept
    {
        auto Iter = m_MetaData.find("Guid");
        return Iter != m_MetaData.not_found() ? Handle::FromString(Iter->second.get_value<std::string>()) : Handle::Null;
    }

    StringU8 AssetMetaData::GetHash() const noexcept
    {
        auto Iter = m_MetaData.find("Hash");
        return Iter != m_MetaData.not_found() ? Iter->second.get_value<std::string>() : StringU8();
    }

    boost::property_tree::ptree& AssetMetaData::GetLoaderData() noexcept
    {
        return *m_LoaderData;
    }
} // namespace Neon::AAsset