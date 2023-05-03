#pragma once

#include <Core/Neon.hpp>

namespace Neon::Asset
{
    class IAssetResource;

    class IAssetResourceHandler
    {
    public:
        /// <summary>
        /// Cast resource to validate if the handler can load/save it.
        /// </summary>
        virtual bool CanCastTo(
            const Ptr<IAssetResource>& Resource) = 0;

        /// <summary>
        /// Get resource size for saving.
        /// </summary>
        virtual size_t QuerySize(
            const Ptr<IAssetResource>& Resource) = 0;

        /// <summary>
        /// Load data from bytes.
        /// </summary>
        [[nodiscard]] virtual Ptr<IAssetResource> Load(
            const uint8_t* Data,
            size_t         DataSize) = 0;

        /// <summary>
        /// Save data to bytes.
        /// </summary>
        [[nodiscard]] virtual void Save(
            const Ptr<IAssetResource>& Resource,
            uint8_t*                   Data,
            size_t                     DataSize) = 0;
    };
} // namespace Neon::Asset