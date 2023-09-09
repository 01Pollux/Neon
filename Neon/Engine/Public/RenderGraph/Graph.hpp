#pragma once

#include <flecs/flecs.h>

#include <RenderGraph/Storage.hpp>
#include <RenderGraph/Pass.hpp>
#include <RenderGraph/BackBuffer.hpp>

#include <RHI/Commands/Context.hpp>

#include <Asio/ThreadPool.hpp>

namespace Neon::Scene::Component
{
    struct Camera;
    struct Transform;
} // namespace Neon::Scene::Component

namespace Neon::RG
{
    class IRenderPass;
    class GraphBuilder;
    class GraphDepdencyLevel;

    class RenderGraph
    {
        friend class GraphDepdencyLevel;

        friend class GraphBuilder;
        using DepdencyLevelList = std::vector<GraphDepdencyLevel>;

        struct ChainedCommandList
        {
            RHI::ICommonCommandList* CommandList = nullptr;
            bool                     IsDirect    = true;

            RHI::GraphicsCommandContext RenderContext;
            RHI::ComputeCommandContext  ComputeContext;

            template<bool _IsDirect>
            [[nodiscard]] auto NewCommandList()
            {
                if constexpr (_IsDirect)
                {
                    return RenderContext.Append();
                }
                else
                {
                    return ComputeContext.Append();
                }
            }

            /// <summary>
            /// Get the previously available command list or append new one
            /// </summary>
            [[nodiscard]] RHI::ICommonCommandList* Load(
                bool Direct);

            /// <summary>
            /// Prepare the command list for recording
            /// </summary>
            [[nodiscard]] void Preload(
                bool IsDirect);

            /// <summary>
            /// Flush the command list to the queue
            /// </summary>
            void Flush();

            /// <summary>
            /// Flush if there are more than 2 command lists, else delay for next execution
            /// </summary>
            void FlushOrDelay();
        };

    public:
        /// <summary>
        /// Reset resource graph for recording
        /// </summary>
        [[nodiscard]] GraphBuilder Reset();

        /// <summary>
        /// Get the storage of the graph
        /// </summary>
        [[nodiscard]] GraphStorage& GetStorage() noexcept;

        /// <summary>
        /// Get the storage of the graph
        /// </summary>
        [[nodiscard]] const GraphStorage& GetStorage() const noexcept;

        /// <summary>
        /// Update camera buffer
        /// </summary>
        void Update(
            const Scene::Component::Camera&    Camera,
            const Scene::Component::Transform& Transform);

        /// <summary>
        /// Run the graph
        /// For convinience, the graph will copy the final output to the back buffer
        /// The graph will also return the command list for us to handle ui rendering
        /// If CopyToBackBuffer is false, the function will return nullptr
        /// </summary>
        void Draw(
            bool CopyToBackBuffer = false);

    private:
        /// <summary>
        /// Build levels and imported resources
        /// </summary>
        void Build(
            DepdencyLevelList&& Levels);

    private:
        GraphStorage      m_Storage;
        DepdencyLevelList m_Levels;

#ifndef NEON_EDITOR
        BackBufferFinalizer m_BackBufferFinalizer;
#endif

        std::mutex         m_RenderMutex, m_ComputeMutex;
        Asio::ThreadPool<> m_ThreadPool{ 3 };
    };

    //

    class GraphDepdencyLevel
    {

    public:
        GraphDepdencyLevel(
            RenderGraph& Context);

        GraphDepdencyLevel(const GraphDepdencyLevel&)            = delete;
        GraphDepdencyLevel(GraphDepdencyLevel&&)                 = default;
        GraphDepdencyLevel& operator=(const GraphDepdencyLevel&) = delete;
        GraphDepdencyLevel& operator=(GraphDepdencyLevel&&)      = default;
        ~GraphDepdencyLevel()                                    = default;

        /// <summary>
        /// Append render pass
        /// </summary>
        void AddPass(
            UPtr<IRenderPass>                             Pass,
            std::vector<ResourceViewId>                   RenderTargets,
            std::optional<ResourceViewId>                 DepthStencil,
            std::set<ResourceId>                          ResourceToCreate,
            std::set<ResourceId>                          ResourceToDestroy,
            std::map<ResourceViewId, RHI::MResourceState> States);

        /// <summary>
        /// Execute render passes
        /// </summary>
        void Execute(
            RenderGraph::ChainedCommandList& ChainedCommandList) const;

    private:
        /// <summary>
        /// Execute pending resource barriers before render passes
        /// </summary>
        void ExecuteBarriers(
            RenderGraph::ChainedCommandList& ChainedCommandList) const;

        /// <summary>
        /// Execute render passes
        /// </summary>
        void ExecutePasses(
            RenderGraph::ChainedCommandList& ChainedCommandList) const;

    private:
        struct RenderPassInfo
        {
            UPtr<IRenderPass>             Pass;
            std::vector<ResourceViewId>   RenderTargets;
            std::optional<ResourceViewId> DepthStencil;
        };

        RenderGraph& m_Context;

        std::vector<RenderPassInfo> m_Passes;

        std::set<ResourceId> m_ResourcesToCreate;
        std::set<ResourceId> m_ResourcesToDestroy;

        std::map<ResourceViewId, RHI::MResourceState> m_States;
    };
} // namespace Neon::RG