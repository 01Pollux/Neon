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
            std::ifstream& Stream);

        /// <summary>
        /// Get the paths of the assets in this package.
        /// </summary>
        [[nodiscard]] PathHandleGenerator GetPaths() const noexcept;

        /// <summary>
        /// Append a path to this package.
        /// </summary>
        void Append(
            const Handle&   Handle,
            const StringU8& HandlerName,
            const StringU8& Path);

        /// <summary>
        /// Save the package descriptor to the file.
        /// </summary>
        void Save(
            std::ofstream& Stream);

    private:
        boost::property_tree::ptree m_Tree;
    };
} // namespace Neon::AAsset