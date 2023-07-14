#pragma once

#include <boost/property_tree/ptree.hpp>
#include <cppcoro/generator.hpp>
#include <fstream>
#include <Asset/Handle.hpp>

namespace Neon::AAsset
{
    class PackageDescriptor
    {
    public:
        using PathHandleGenerator = cppcoro::generator<std::pair<Handle, std::string>>;

        PackageDescriptor(
            std::ifstream Stream);

        PathHandleGenerator GetPaths() const noexcept;

    private:
        boost::property_tree::ptree m_Tree;
    };
} // namespace Neon::AAsset