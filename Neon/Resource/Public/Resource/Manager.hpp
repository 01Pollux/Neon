#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>

#include <Resource/Pack.hpp>
#include <Resource/Handler.hpp>

namespace Neon::Asset
{
    class IResourceManager
    {
        using AssetPackMap = std::map<StringU8, Ptr<IAssetPack>>;

    public:
        static constexpr const char* Global_RuntimePack = "_runtime.np";
        static constexpr const char* Global_EditorPack  = "_editor.np";

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
        /// Load pack file
        /// </summary>
        void LoadPack(
            const StringU8& Path,
            const StringU8& Tag);

        /// <summary>
        /// Unload pack file
        /// </summary>
        void UnloadPack(
            const StringU8& Tag);

    public:
        void ImportPack(
            const StringU8& Tag,
            const StringU8& FilePath);

        void ExportPack(
            const StringU8& Tag,
            const StringU8& FilePath);

        Ref<IAssetResource> LoadPack(
            const StringU8&    Tag,
            const AssetHandle& Handle);

        void SavePack(
            const StringU8&            Tag,
            const AssetHandle&         Handle,
            const Ptr<IAssetResource>& Resource);

    protected:
        /// <summary>
        /// Open pack file from path
        /// </summary>
        [[nodiscard]] virtual Ptr<IAssetPack> OpenPack(
            const StringU8& Path) = 0;

    private:
        /// <summary>
        /// Get pack from tag.
        /// </summary>
        [[nodiscard]] IAssetPack* GetPack(
            const StringU8& Tag);

    private:
        AssetResourceHandlers m_Handlers;
        AssetPackMap          m_LoadedPacks;
    };
} // namespace Neon::Asset