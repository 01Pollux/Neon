#pragma once

namespace Neon::Runtime
{
    class EnginePipelineBuilder;
    class DefaultGameEngine;
} // namespace Neon::Runtime

namespace Neon::Runtime::Phases
{
    class SplashScreen
    {
    public:
        static void Build(
            EnginePipelineBuilder& Builder);

        static void Bind(
            DefaultGameEngine* Engine);
    };
} // namespace Neon::Runtime::Phases