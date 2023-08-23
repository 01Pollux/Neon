#pragma once

#include <Core/String.hpp>
#include <flecs/flecs.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

namespace Neon::Editor
{
    class IEditorComponentHandler
    {
    public:
        virtual ~IEditorComponentHandler() = default;

        /// <summary>
        /// Called when the component is being handled.
        /// return true if the component is handled and stop the dispatching.s
        /// </summary>
        virtual bool Dispatch(
            const flecs::entity& Entity,
            const flecs::id&     ComponentId) = 0;
    };
} // namespace Neon::Editor