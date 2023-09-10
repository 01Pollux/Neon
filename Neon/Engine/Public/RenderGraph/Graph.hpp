#pragma once

#include <RenderGraph/Storage.hpp>
#include <RenderGraph/Pass.hpp>

#include <RHI/Fence.hpp>
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

        class CommandListContext
        {
        public:
            CommandListContext() = default;

            CommandListContext(
                uint32_t MaxGraphicsCount,
                uint32_t MaxComputeCount);

            /// <summary>
            /// Begin command list context
            /// </summary>
            void Begin();

            /// <summary>
            /// Flush command lists
            /// </summary>
            void Flush(
                size_t GraphicsCount,
                size_t ComputeCount,
                bool   Reset);

            /// <summary>
            /// End command list context
            /// </summary>
            void End();

            /// <summary>
            /// Get graphics command list
            /// </summary>
            [[nodiscard]] RHI::IGraphicsCommandList* GetGraphics(
                size_t Index);

            /// <summary>
            /// Get graphics command list
            /// </summary>
            [[nodiscard]] RHI::IComputeCommandList* GetCompute(
                size_t Index);

            /// <summary>
            /// Get graphics command list count
            /// </summary>
            [[nodiscard]] size_t GetGraphicsCount() const noexcept;

            /// <summary>
            /// Get compute command list count
            /// </summary>
            [[nodiscard]] size_t GetComputeCount() const noexcept;

            /// <summary>
            /// Wait for the fence
            /// </summary>
            void Wait(
                size_t Value);

            /// <summary>
            /// Signal the fence
            /// </summary>
            void Signal(
                size_t Value);

        private:
            UPtr<RHI::IFence> m_Fence;

            std::vector<RHI::IGraphicsCommandList*> m_GraphicsCommandList;
            std::vector<RHI::IComputeCommandList*>  m_ComputeCommandList;
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
        /// </summary>
        void Draw();

    private:
        /// <summary>
        /// Build levels and imported resources
        /// </summary>
        void Build(
            DepdencyLevelList&& Levels);

    private:
        GraphStorage      m_Storage;
        DepdencyLevelList m_Levels;

        CommandListContext m_CommandListContext;

        std::mutex         m_RenderMutex, m_ComputeMutex;
        Asio::ThreadPool<> m_ThreadPool{ 3 };
    };

    //

    class GraphDepdencyLevel
    {
        friend class RenderGraph;

        struct RenderPassInfo
        {
            UPtr<IRenderPass>             Pass;
            std::vector<ResourceViewId>   RenderTargets;
            std::optional<ResourceViewId> DepthStencil;
        };

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
            bool Reset) const;

    private:
        /// <summary>
        /// Execute pending resource barriers before render passes
        /// </summary>
        void ExecuteBarriers() const;

        /// <summary>
        /// Execute render passes
        /// </summary>
        void ExecutePasses() const;

    private:
        RenderGraph& m_Context;

        std::vector<RenderPassInfo> m_Passes;

        std::set<ResourceId> m_ResourcesToCreate;
        std::set<ResourceId> m_ResourcesToDestroy;

        std::map<ResourceViewId, RHI::MResourceState> m_States;

        uint32_t m_GraphicsCount = 0,
                 m_ComputeCount  = 0;

        bool m_FlushCommands : 1 = false;
    };
} // namespace Neon::RG