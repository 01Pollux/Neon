#include <ResourcePCH.hpp>
#include <Asset/PackageDescriptor.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace Neon::AAsset
{
    PackageDescriptor::PackageDescriptor(
        std::ifstream Stream)
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
} // namespace Neon::AAsset