#pragma once

#include <Runtime/Runtime.hpp>

namespace Neon::Runtime
{
    class LoadingScreenRuntime : public EngineRuntime
    {
    public:
        LoadingScreenRuntime(
            DefaultGameEngine* Engine);

    private:
        /// <summary>
        /// Setup the rendergraph for the loading screen.
        /// </summary>
        void SetupRendergraph(
            DefaultGameEngine* Engine);
    };
} // namespace Neon::Runtime