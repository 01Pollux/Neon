#include <ResourcePCH.hpp>
#include <Asset/PackageDescriptor.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace Neon::AAsset
{
    PackageDescriptor::PackageDescriptor(
        std::ifstream& Stream)
    {
        boost::property_tree::read_ini(Stream, m_Tree);
    }

    auto PackageDescriptor::GetPaths() const noexcept -> PathHandleGenerator
    {
        for (const auto& [Key, Value] : m_Tree)
        {
            co_yield PathHandleInfo{
                .Handle      = Handle::FromString(Key),
                .HandlerName = Value.get_child("Handler").get_value<StringU8>(),
                .Path        = Value.get_child("Path").get_value<StringU8>()
            };
        }
    }
    void PackageDescriptor::Append(
        const Handle&   Handle,
        const StringU8& HandlerName,
        const StringU8& Path)
    {
        boost::property_tree::ptree Node;
        Node.push_back({ "Handler", boost::property_tree::ptree(HandlerName) });
        Node.push_back({ "Path", boost::property_tree::ptree(Path) });
        m_Tree.push_back({ Handle.ToString(), std::move(Node) });
    }

    void PackageDescriptor::Save(
        std::ofstream& Stream)
    {
        boost::property_tree::write_ini(Stream, m_Tree);
    }
} // namespace Neon::AAsset