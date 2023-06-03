#pragma once

#include <Core/Neon.hpp>
#include <Resource/Handle.hpp>
#include <IO/BinaryFile.hpp>
#include <map>

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
            IO::BinaryStreamReader Stream,
            size_t                 DataSize) = 0;

        /// <summary>
        /// Save data to bytes.
        /// </summary>
        [[nodiscard]] virtual void Save(
            const Ptr<IAssetResource>& Resource,
            IO::BinaryStreamWriter     Stream,
            size_t                     DataSize) = 0;
    };

    class AssetResourceHandlers
    {
    public:
        /// <summary>
        /// Add resource handler
        /// </summary>
        void Append(
            UPtr<IAssetResourceHandler> Handler);

        /// <summary>
        /// Get resource handler
        /// </summary>
        [[nodiscard]] IAssetResourceHandler* Get(
            size_t LoaderId) const noexcept;

        /// <summary>
        /// Get current handlers
        /// </summary>
        [[nodiscard]] const auto& Get() const noexcept
        {
            return m_Handlers;
        }

    private:
        std::map<size_t, UPtr<IAssetResourceHandler>> m_Handlers;
    };
} // namespace Neon::Asset