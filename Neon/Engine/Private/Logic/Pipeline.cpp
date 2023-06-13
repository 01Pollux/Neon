#include <EnginePCH.hpp>
#include <Logic/Pipeline.hpp>

namespace Neon
{
    Pipeline::Pipeline() :
        m_ThreadCount(std::thread::hardware_concurrency() / 2)
    {
    }

    void Pipeline::Dispatch()
    {
    }

    void Pipeline::SetThreadCount(uint16_t ThreadCount)
    {
    }

    void Pipeline::SetPhaseEnable(const StringU8& PhaseName, bool State)
    {
    }

    void Pipeline::SetPhaseParallelize(const StringU8& PhaseName, bool State)
    {
    }
} // namespace Neon