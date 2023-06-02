#pragma once

#include <Resource/Handler.hpp>
#include <Resource/Operator.hpp>

namespace Neon::Asset
{
    class IResourceManager
    {
        using AssetPackMap = std::map<StringU8, UPtr<IAssetPack>>;

    public:
        virtual ~IResourceManager() = default;

    public:
        /// <summary>
        /// Append resource handler
        /// </summary>
        void AddHandler(
            UPtr<IAssetResourceHandler> Handler);

        /// <summary>
        /// Append resource handler
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::is_base_of_v<IAssetResourceHandler, _Ty>
        void AddHandler(
            _Args&&... Args)
        {
            AddHandler(std::make_unique<_Ty>(std::forward<_Args>(Args)...));
        }

    public:
        /// <summary>
        /// Create empty pack file
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::is_base_of_v<IAssetPack, _Ty>
        [[nodiscard]] _Ty* NewPack(
            const StringU8& Tag,
            _Args&&... Args)
        {
            PackIsUnique(Tag);
            auto& Pack = m_LoadedPacks.emplace(Tag, InstantiatePack<_Ty>(std::forward<_Args>(Args)...)).first->second;
            return static_cast<_Ty*>(Pack.get());
        }

        /// <summary>
        /// Get current loaded packs.
        /// </summary>
        [[nodiscard]] const AssetPackMap& GetPacks() const;

        /// <summary>
        /// Get pack from tag.
        /// </summary>
        [[nodiscard]] IAssetPack* GetPack(
            const StringU8& Tag);

        /// <summary>
        /// Load pack file
        /// </summary>
        IAssetPack* LoadPack(
            const StringU8& Tag,
            const StringU8& Path);

        /// <summary>
        /// Unload pack file
        /// </summary>
        void UnloadPack(
            const StringU8& Tag);

    protected:
        /// <summary>
        /// Open pack file from path
        /// </summary>
        [[nodiscard]] virtual UPtr<IAssetPack> OpenPack(
            const StringU8& Path) = 0;

        template<typename _Ty, typename... _Args>
            requires std::is_base_of_v<IAssetPack, _Ty>
        [[nodiscard]] UPtr<IAssetPack> InstantiatePack(
            _Args&&... Args)
        {
            return UPtr<IAssetPack>{ std::make_unique<_Ty>(m_Handlers, m_DeferredResourceOperator, std::forward<_Args>(Args)...) };
        }

    private:
        /// <summary>
        /// Validate if the pack is unique
        /// </summary>
        void PackIsUnique(
            const StringU8& Tag) const;

    private:
        AssetResourceHandlers   m_Handlers;
        AssetPackMap            m_LoadedPacks;
        DeferredResourceOperator m_DeferredResourceOperator;
    };
} // namespace Neon::Asset