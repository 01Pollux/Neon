#include <EnginePCH.hpp>
#include <Editor/Profile/Profile.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Asset/Storage.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Handlers/Texture.hpp>

#include <RHI/GlobalDescriptors.hpp>
#include <RHI/Resource/Views/ShaderResource.hpp>

namespace Neon::Editor
{
    Profile::Profile(
        const StringU8& Path)
    {
    }

    ImTextureID Profile::LoadTexture(
        const StringU8& TexturePath) const
    {
        auto Guid = Asset::Handle::FromString(TexturePath);
        if (auto Id = m_LoadedTextures.find(Guid); Id != m_LoadedTextures.end())
        {
            return Id->second;
        }

        auto Texture = Asset::AssetTaskPtr<Asset::TextureAsset>(Asset::Manager::Load(Guid))->GetTexture();

        auto Descriptor = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
        auto Handle     = Descriptor->Allocate(1);

        RHI::Views::ShaderResource View(Handle);
        View.Bind(Texture.get());

        return m_LoadedTextures[Guid] = std::bit_cast<ImTextureID>(Handle.GetGpuHandle());
    }
} // namespace Neon::Editor