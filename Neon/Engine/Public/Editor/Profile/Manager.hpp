#pragma once

#include <Editor/Profile/Profile.hpp>

namespace Neon::Editor
{
    class ProfileManager
    {
    public:
        /// <summary>
        /// Loads the profile.
        /// </summary>
        static void Load(
            const Asset::Handle& ProfileGuid);

        /// <summary>
        /// Unloads the profile.
        /// </summary>
        static void Shutdown();

        /// <summary>
        /// Gets the active profile in current context.
        /// </summary>
        static Profile* Get();
    };
} // namespace Neon::Editor