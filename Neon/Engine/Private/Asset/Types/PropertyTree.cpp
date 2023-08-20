#include <EnginePCH.hpp>
#include <Asset/Handlers/PropertyTree.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace Neon::Asset
{
    void PropertyTreeAsset::Set(
        boost::property_tree::ptree Tree)
    {
        MarkDirty();
        m_Tree = std::move(Tree);
    }

    //

    bool PropertyTreeAsset::Handler::CanHandle(
        const Ptr<IAsset>& Asset)
    {
        return dynamic_cast<PropertyTreeAsset*>(Asset.get());
    }

    Ptr<IAsset> PropertyTreeAsset::Handler::Load(
        std::istream& Stream,
        const Asset::DependencyReader&,
        const Handle&        AssetGuid,
        StringU8             Path,
        const AssetMetaData& LoaderData)
    {
        auto Asset = std::make_shared<PropertyTreeAsset>(AssetGuid, std::move(Path));

        PropertyTreeAsset::TreeType Type = PropertyTreeAsset::TreeType::Json;
        if (auto TreeType = LoaderData.get_child_optional("Type"))
        {
            if (TreeType->get_value<std::string>() == "Xml")
            {
                Type = PropertyTreeAsset::TreeType::Xml;
            }
            else if (TreeType->get_value<std::string>() == "Ini")
            {
                Type = PropertyTreeAsset::TreeType::Ini;
            }
        }

        Asset->m_Type = Type;

        switch (Type)
        {
        case PropertyTreeAsset::TreeType::Json:
        {
            boost::property_tree::read_json(Stream, Asset->m_Tree);
            break;
        }
        case PropertyTreeAsset::TreeType::Xml:
        {
            boost::property_tree::read_xml(Stream, Asset->m_Tree);
            break;
        }
        case PropertyTreeAsset::TreeType::Ini:
        {
            boost::property_tree::read_ini(Stream, Asset->m_Tree);
            break;
        }
        }

        return Asset;
    }

    void PropertyTreeAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto AssetPtr = static_cast<Asset::PropertyTreeAsset*>(Asset.get());

        switch (AssetPtr->m_Type)
        {
        case PropertyTreeAsset::TreeType::Json:
        {
            boost::property_tree::write_json(Stream, AssetPtr->m_Tree);
            break;
        }
        case PropertyTreeAsset::TreeType::Xml:
        {
            boost::property_tree::write_xml(Stream, AssetPtr->m_Tree);
            break;
        }
        case PropertyTreeAsset::TreeType::Ini:
        {
            boost::property_tree::write_ini(Stream, AssetPtr->m_Tree);
            break;
        }
        default:
            std::unreachable();
        }
    }
} // namespace Neon::Asset