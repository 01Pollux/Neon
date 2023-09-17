#include <EnginePCH.hpp>
#include <Editor/Profile/Profile.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Asset/Storage.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Handlers/Texture.hpp>
#include <Asset/Handlers/PropertyTree.hpp>

#include <RHI/GlobalDescriptors.hpp>

namespace Neon::Editor
{
    Profile::Profile(
        const Asset::Handle& ProfileAsset) :
        m_Properties(std::dynamic_pointer_cast<Asset::PropertyTreeAsset>(Asset::Manager::Load(ProfileAsset))->Get())
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

        auto& Texture = std::dynamic_pointer_cast<Asset::TextureAsset>(Asset::Manager::Load(Guid))->GetTexture();

        auto Descriptor = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
        auto Handle     = Descriptor->Allocate(1);

        Handle->CreateShaderResourceView(
            Handle.Offset, Texture.Get().get());

        return m_LoadedTextures[Guid] = std::bit_cast<ImTextureID>(Handle.GetCpuHandle());
    }
} // namespace Neon::Editor