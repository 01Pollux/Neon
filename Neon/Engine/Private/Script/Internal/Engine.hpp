#pragma once

#include <Script/Engine.hpp>
#include <Private/Script/HandleManager.hpp>
#include <Private/Script/Internal/Assembly.hpp>

typedef struct _MonoDomain MonoDomain;

namespace Neon::Scripting::CS
{
    struct ScriptContext
    {
    public:
        /// <summary>
        /// Get the main script context.
        /// </summary>
        [[nodiscard]] static ScriptContext* Get();

        /// <summary>
        /// Create a new domain.
        /// </summary>
        void NewDomain();

    public:
        MonoDomain* RootDomain{};
        MonoDomain* CurrentDomain{};

        HandleManager HandleMgr;

        std::map<StringU8, Assembly> LoadedAssemblies;

        bool IsMonoInitialized : 1 = false;
    };
} // namespace Neon::Scripting::CS