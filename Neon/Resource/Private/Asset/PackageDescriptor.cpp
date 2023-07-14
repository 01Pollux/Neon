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
            co_yield std::make_pair(
                Handle::FromString(Key),
                Value.get_value<std::string>());
        }
    }
} // namespace Neon::AAsset