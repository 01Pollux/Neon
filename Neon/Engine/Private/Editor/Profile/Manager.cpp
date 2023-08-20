#include <EnginePCH.hpp>
#include <Editor/Profile/Manager.hpp>

namespace Neon::Editor
{
    static UPtr<Profile> s_ActiveProfile;

    void ProfileManager::Load(
        const Asset::Handle& ProfileGuid)
    {
        s_ActiveProfile = std::make_unique<Profile>(ProfileGuid);
    }

    void ProfileManager::Shutdown()
    {
        s_ActiveProfile = nullptr;
    }

    Profile* ProfileManager::Get()
    {
        return s_ActiveProfile.get();
    }
} // namespace Neon::Editor