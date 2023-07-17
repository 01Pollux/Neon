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
        struct PathHandleInfo
        {
            Handle   Handle;
            StringU8 HandlerName;
            StringU8 Path;
        };

        using PathHandleGenerator = cppcoro::generator<PathHandleInfo>;

        PackageDescriptor() = default;

        PackageDescriptor(
            std::ifstream Stream);

        PathHandleGenerator GetPaths() const noexcept;

    private:
        boost::property_tree::ptree m_Tree;
    };
} // namespace Neon::AAsset