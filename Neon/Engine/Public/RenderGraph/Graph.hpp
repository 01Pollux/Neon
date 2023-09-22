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
            /// Preallocate command lists
            /// </summary>
            void Begin();

            /// <summary>
            /// Flush command lists
            /// </summary>
            void Flush(
                size_t GraphicsCount,
                size_t ComputeCount);

            /// <summary>
            /// reset command lists with new allocators
            /// </summary>
            void Reset(
                size_t GraphicsCount,
                size_t ComputeCount);

            /// <summary>
            /// Free allocated command lists
            /// </summary>
            void End();

            /// <summary>
            /// Get graphics command list
            /// </summary>
            [[nodiscard]] RHI::ICommandList* GetGraphics(
                size_t Index);

            /// <summary>
            /// Get graphics command list
            /// </summary>
            [[nodiscard]] RHI::ICommandList* GetCompute(
                size_t Index);

            /// <summary>
            /// Get graphics command list count
            /// </summary>
            [[nodiscard]] size_t GetGraphicsCount() const noexcept;

            /// <summary>
            /// Get compute command list count
            /// </summary>
            [[nodiscard]] size_t GetComputeCount() const noexcept;

        private:
            UPtr<RHI::IFence> m_Fence;
            uint64_t          m_FenceValue = 0;

            std::vector<RHI::ICommandList*> m_GraphicsCommandList;
            std::vector<RHI::ICommandList*> m_ComputeCommandList;
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
            std::map<ResourceViewId, RHI::MResourceState> States);

        /// <summary>
        /// Execute render passes
        /// </summary>
        void Execute() const;

        /// <summary>
        /// Get command list count for this level
        /// </summary>
        [[nodiscard]] std::pair<uint32_t, uint32_t> GetCommandListCount() const;

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

        /// <summary>
        /// Used when the previous level have executed N commands, where we need to use N+M commands
        /// Check CommandListContext for more details
        /// </summary>
        uint32_t m_GraphicsCommandsToReserve = 0;
        uint32_t m_GraphicsCommandsToFlush   = 0;

        /// <summary>
        /// Used when the previous level have executed N commands, where we need to use N+M commands
        /// Check CommandListContext for more details
        /// </summary>
        uint32_t m_ComputeCommandsToReserve = 0;
        uint32_t m_ComputeCommandsToFlush   = 0;

        std::vector<RenderPassInfo> m_Passes;

        std::set<ResourceId> m_ResourcesToCreate;

        std::map<ResourceId, std::map<uint32_t, RHI::MResourceState>> m_StatesToTransition;

        bool m_ResetBarriers : 1 = false;
        bool m_ResetCommands : 1 = false;

        bool m_FlushBarriers : 1 = false;
        bool m_FlushCommands : 1 = false;
    };
} // namespace Neon::RG