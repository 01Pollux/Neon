#pragma once

#include <Resource/Handler.hpp>
#include <Resource/Operator.hpp>

namespace Neon::Asset
{
    class IAssetManager
    {
        using AssetPackMap = std::map<StringU8, Ptr<IAssetPack>>;

    public:
        virtual ~IAssetManager() = default;

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
        [[nodiscard]] Ptr<_Ty> NewPack(
            const StringU8& Tag,
            _Args&&... Args)
        {
            PackIsUnique(Tag);
            auto& Pack = m_LoadedPacks.emplace(Tag, InstantiatePack<_Ty>(std::forward<_Args>(Args)...)).first->second;
            return std::static_pointer_cast<_Ty>(Pack);
        }

        /// <summary>
        /// Get current loaded packs.
        /// </summary>
        [[nodiscard]] const AssetPackMap& GetPacks() const;

        /// <summary>
        /// Get pack from tag.
        /// </summary>
        [[nodiscard]] const Ptr<IAssetPack>& GetPack(
            const StringU8& Tag);

        /// <summary>
        /// Load pack file
        /// </summary>
        Ptr<IAssetPack> TryLoadPack(
            const StringU8& Tag,
            const StringU8& Path);

        /// <summary>
        /// Load pack file
        /// </summary>
        Ptr<IAssetPack> LoadPack(
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
        [[nodiscard]] virtual Ptr<IAssetPack> OpenPack(
            const StringU8& Path) = 0;

        template<typename _Ty, typename... _Args>
            requires std::is_base_of_v<IAssetPack, _Ty>
        [[nodiscard]] Ptr<IAssetPack> InstantiatePack(
            _Args&&... Args)
        {
            return Ptr<IAssetPack>{ NEON_NEW _Ty(m_Handlers, m_DeferredResourceOperator, std::forward<_Args>(Args)...) };
        }

    private:
        /// <summary>
        /// Validate if the pack is unique
        /// </summary>
        void PackIsUnique(
            const StringU8& Tag) const;

    private:
        AssetResourceHandlers    m_Handlers;
        AssetPackMap             m_LoadedPacks;
        DeferredResourceOperator m_DeferredResourceOperator;
    };
} // namespace Neon::Asset