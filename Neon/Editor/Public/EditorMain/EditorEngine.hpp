#pragma once

#include <Runtime/GameEngine.hpp>

namespace Neon::Editor
{
    class EditorEngine : public Runtime::DefaultGameEngine
    {
    public:
        void Initialize(
            Config::EngineConfig Config) override;

    private:
    };
} // namespace Neon::Editor