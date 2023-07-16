#pragma once

#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>
#include <vector>

namespace Neon::AAsset
{
    class Storage;

    class AssetDependencyGraph
    {
    public:
        struct BuildTask
        {
            /// <summary>
            /// Resolve the task by loading the asset.
            /// </summary>
            Asio::CoLazy<> Resolve(
                Storage* AssetStorage)
            {
                co_return;
            }
        };

        /// <summary>
        /// Compile the graph and return a generator of build tasks.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<BuildTask> Compile()
        {
            co_yield {};
        }

        /// <summary>
        /// Add a dependency between two assets.
        /// </summary>
        void Requires(
            const Handle& Parent,
            const Handle& Child)
        {
        }

        /// <summary>
        ///
        /// </summary>
        template<typename _Ty>
        void Requires()
        {
        }

    private:
        struct BuildNode
        {
            Handle                  ResHandle;
            std::vector<BuildNode*> Dependencies;
        };

        std::vector<BuildNode> m_BuildNodes;
    };
} // namespace Neon::AAsset