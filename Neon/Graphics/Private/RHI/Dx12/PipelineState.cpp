#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/PipelineState.hpp>

namespace Neon::RHI
{
    std::mutex                                   s_PipelineStateCacheMutex;
    std::map<SHA256::Bytes, Ptr<IRootSignature>> s_PipelineStateCache;

    //

    Ptr<IPipelineState> IPipelineState::Create(
        const PipelineStateBuilder& Builder)
    {
        return Dx12PipelineStateCache::Load(Builder);
    }
    Ptr<IPipelineState> IPipelineState::Create(
        const ComputePipelineStateBuilder& Builder)
    {
        return Ptr<IPipelineState>();
    }

    void Dx12PipelineStateCache::Flush()
    {
        std::scoped_lock Lock(s_PipelineStateCacheMutex);
        s_PipelineStateCache.clear();
    }

    Ptr<IPipelineState> Dx12PipelineStateCache::Load(
        const PipelineStateBuilder& Builder)
    {
        std::scoped_lock Lock(s_PipelineStateCacheMutex);
    }

    Ptr<IPipelineState> Dx12PipelineStateCache::Load(
        const ComputePipelineStateBuilder& Builder)
    {
        std::scoped_lock Lock(s_PipelineStateCacheMutex);
    }
} // namespace Neon::RHI