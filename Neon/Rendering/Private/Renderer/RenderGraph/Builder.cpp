#include <EnginePCH.hpp>
#include <Renderer/RenderGraph/Builder.hpp>
#include <Renderer/RenderGraph/Context.hpp>
#include <Renderer/RenderGraph/Builder.hpp>

#include <Renderer/RGPasses/Backbuffer.hpp>

#include <Resource/Handler/BinaryResourceFile.hpp>
#include <Resource/ResourceManager.hpp>

#include <RHI/RenderDevice.hpp>

namespace Renderer::RG
{
    RenderGraphBuilder::RenderGraphBuilder(
        RenderGraphContext& Context) :
        m_Context(Context)
    {
        AppendPass<InitializeBackbufferPass>();
    }

    void RenderGraphBuilder::Build()
    {
         AppendPass<FinalizeBackbufferPass>();

        //

        auto LoadedShaders = std::make_shared<LockableData<ShaderMapType>>();
        auto Builders      = std::make_shared<BuildersListType>();

        Builders->reserve(m_Passes.size());
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            Builders->emplace_back(m_Context.GetStorage(), LoadedShaders->GetUnsafe());
        }

        // Launch pipeline state jobs
        m_Context.m_PipelineCreators = LaunchPipelineJobs(Builders, LoadedShaders);

        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            auto& Builder = (*Builders)[i];
            m_Passes[i]->SetupResources(Builder.Resources);
        }
        m_Context.Build(BuildPasses(Builders));
    }

    //

    DXGI_FORMAT RenderGraphBuilder::GetBackbufferFormat()
    {
        return RHI::IRenderDevice::SwapchainFormat;
    }

    std::vector<std::future<void>> RenderGraphBuilder::LaunchRootSignatureJobs(
        const std::shared_ptr<BuildersListType>& Builders)
    {
        std::vector<std::future<void>> Jobs;
        Jobs.reserve(m_Passes.size());

        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            auto& Builder = (*Builders)[i];
            m_Passes[i]->SetupRootSignatures(Builder.RootSignatures);
        }

        for (size_t i = 0; i < Builders->size(); i++)
        {
            auto& Builder = (*Builders)[i];
            for (auto& [Id, Desc] : Builder.RootSignatures.m_RootSignaturesToLoad)
            {
                auto RootSignatureJob =
                    [Desc     = std::move(Desc),
                     &Storage = m_Context.GetStorage(),
                     Id,
                     Builders]()
                {
                    Storage.ImportRootSignature(Id, RHI::RootSignature(Desc.Build()));
                };

                Jobs.emplace_back(std::async(std::move(RootSignatureJob)));
            }
        }
        return Jobs;
    }

    //

    std::vector<std::future<void>> RenderGraphBuilder::LaunchShaderJobs(
        const std::shared_ptr<BuildersListType>&            Builders,
        const std::shared_ptr<LockableData<ShaderMapType>>& LoadedShaders) const
    {
        std::vector<std::future<void>> Jobs;
        Jobs.reserve(m_Passes.size());

        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            auto& Builder = (*Builders)[i];
            m_Passes[i]->SetupShaders(Builder.Shaders);
        }

        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            auto& Builder = (*Builders)[i];
            for (auto& [Id, ShaderInfo] : Builder.Shaders.m_ShadersToLoad)
            {
                auto ShaderJob = [ShaderInfo = std::move(ShaderInfo),
                                  &Storage   = m_Context.GetStorage(),
                                  Id,
                                  Builders,
                                  LoadedShaders]() mutable
                {
                    RHI::IGraphicsShader::UPtr     Shader;
                    Asset::BinaryResourceFile::Ptr BinaryResource;

                    if (ShaderInfo.IsPath)
                    {
                        Asset::ResourceHandle Handle(ShaderInfo.CodeOrPath);

                        BinaryResource    = Asset::IResourceManager::Get()->GetResourceAs<Asset::BinaryResourceFile>(Handle);
                        auto ResourceView = BinaryResource->Get().view();

                        ShaderInfo.Desc.ShaderCode = StringU8View(std::bit_cast<const char*>(ResourceView.data()), ResourceView.size());
                    }

                    Shader.reset(RHI::IGraphicsShader::Load(ShaderInfo.Desc));

                    auto Shaders = LoadedShaders->Lock();
                    Shaders->emplace(Id, std::move(Shader));
                };
                Jobs.emplace_back(std::async(std::move(ShaderJob)));
            }
        }
        return Jobs;
    }

    std::future<void> RenderGraphBuilder::LaunchPipelineJobs(
        const std::shared_ptr<BuildersListType>&            Builders,
        const std::shared_ptr<LockableData<ShaderMapType>>& LoadedShaders)
    {
        // Launch root signature and shader jobs
        auto RootSignaturesJobs = LaunchRootSignatureJobs(Builders);
        auto ShaderJobs         = LaunchShaderJobs(Builders, LoadedShaders);

        std::vector<IRenderPass*> PassesCopy(m_Passes.size());
        std::ranges::transform(m_Passes, PassesCopy.begin(), [](auto& Pass)
                               { return Pass.get(); });

        auto PipelineJobDispatcher =
            [PassesCopy         = std::move(PassesCopy),
             RootSignaturesJobs = std::move(RootSignaturesJobs),
             ShaderJobs         = std::move(ShaderJobs),
             &Storage           = m_Context.GetStorage(),
             Builders,
             LoadedShaders]() mutable
        {
            // wait for root signature and shader jobs to finish
            RootSignaturesJobs.clear();
            ShaderJobs.clear();

            // Launch pipeline state jobs for each pipeline state builder
            for (size_t i = 0; i < Builders->size(); i++)
            {
                auto& Builder = (*Builders)[i];
                PassesCopy[i]->SetupPipelineStates(Builder.PipelineStates);
            }

            std::vector<std::future<void>> PipelineStateJobs;
            for (size_t i = 0; i < Builders->size(); i++)
            {
                auto& Builder = (*Builders)[i];
                for (auto& [Id, Desc] : Builder.PipelineStates.m_PipelineStatesToLoad)
                {
                    auto PipelineStateJob =
                        [Id,
                         Desc = std::move(Desc),
                         &Storage]()
                    {
                        if (auto GraphicsDesc = std::get_if<RHI::GraphicsPipelineStateDesc>(&Desc))
                        {
                            Storage.ImportPipelineState(Id, RHI::PipelineState(*GraphicsDesc));
                        }
                        else
                        {
                            Storage.ImportPipelineState(Id, RHI::PipelineState(std::get<RHI::ComputePipelineStateDesc>(Desc)));
                        }
                    };

                    PipelineStateJobs.emplace_back(std::async(PipelineStateJob));
                }
            }
        };

        return std::async(std::move(PipelineJobDispatcher));
    }

    //

    std::vector<RenderGraphContext::DepdencyLevel>
    RenderGraphBuilder::BuildPasses(
        const std::shared_ptr<BuildersListType>& Builders)
    {
        BuildAdjacencyLists(*Builders);
        TopologicalSort();
        auto ResourcesToDestroy = CalculateResourcesLifetime(*Builders);
        return BuildDependencyLevels(*Builders, ResourcesToDestroy);
    }

    //

    void RenderGraphBuilder::BuildAdjacencyLists(
        const BuildersListType& Builders)
    {
        m_AdjacencyList.resize(m_Passes.size());
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            auto& Adjacencies = m_AdjacencyList[i];
            auto& PassBuilder = Builders[i].Resources;

            for (size_t j = i + 1; j < m_Passes.size(); j++)
            {
                auto& OtherPassBuilder = Builders[j].Resources;
                bool  Depends          = false;

                for (auto& OtherPassRead : OtherPassBuilder.m_ResourcesRead)
                {
                    if (PassBuilder.m_ResourcesWritten.contains(OtherPassRead))
                    {
                        Adjacencies.push_back(j);
                        Depends = true;
                        break;
                    }
                }
            }
        }
    }

    //

    void RenderGraphBuilder::TopologicalSort()
    {
        std::stack<size_t> Stack{};
        std::vector<bool>  Visited(m_Passes.size(), false);
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            if (!Visited[i])
            {
                DepthFirstSearch(i, Visited, Stack);
            }
        }

        m_TopologicallySortedList.reserve(Stack.size());
        while (!Stack.empty())
        {
            m_TopologicallySortedList.push_back(Stack.top());
            Stack.pop();
        }
    }

    //

    std::vector<std::set<ResourceId>> RenderGraphBuilder::CalculateResourcesLifetime(
        BuildersListType& Builders) const
    {
        std::vector<std::set<ResourceId>> ResourceToDestroy(m_Passes.size());
        std::map<ResourceId, size_t>      LastUsedResources;

        for (size_t i = 0; i < m_TopologicallySortedList.size(); i++)
        {
            size_t PassIdx   = m_TopologicallySortedList[i];
            auto&  Resources = Builders[PassIdx].Resources;

            for (auto& ResId : Resources.m_ResourcesWritten)
            {
                LastUsedResources[ResId] = PassIdx;
            }
            for (auto& ResId : Resources.m_ResourcesRead)
            {
                LastUsedResources[ResId] = PassIdx;
            }
        }

        for (auto& [Id, PassIdx] : LastUsedResources)
        {
            ResourceToDestroy[PassIdx].insert(Id);
        }

        return ResourceToDestroy;
    }

    //

    void RenderGraphBuilder::DepthFirstSearch(
        size_t              Index,
        std::vector<bool>&  Visited,
        std::stack<size_t>& Stack)
    {
        Visited[Index] = true;
        for (size_t AdjIndex : m_AdjacencyList[Index])
        {
            if (!Visited[AdjIndex])
            {
                DepthFirstSearch(AdjIndex, Visited, Stack);
            }
        }
        Stack.push(Index);
    }

    //

    std::vector<RenderGraphContext::DepdencyLevel>
    RenderGraphBuilder::BuildDependencyLevels(
        BuildersListType&                  Builders,
        std::vector<std::set<ResourceId>>& ResourceToDestroy)
    {
        std::vector<size_t> Distances(m_TopologicallySortedList.size());
        for (size_t d = 0; d < Distances.size(); d++)
        {
            size_t i = m_TopologicallySortedList[d];
            for (size_t AdjIndex : m_AdjacencyList[i])
            {
                if (Distances[AdjIndex] < (Distances[i] + 1))
                {
                    Distances[AdjIndex] = Distances[i] + 1;
                }
            }
        }

        size_t Size = std::ranges::max(Distances) + 1;

        std::vector<RenderGraphContext::DepdencyLevel> Dependencies;
        Dependencies.reserve(Size);

        for (size_t i = 0; i < Size; i++)
        {
            Dependencies.emplace_back(m_Context);
        }

        for (size_t i = 0; i < m_Passes.size(); ++i)
        {
            size_t Level = Distances[i];

            Dependencies[Level].AddPass(
                std::move(m_Passes[i]),
                std::move(Builders[i].Resources.m_RenderTargets),
                std::move(Builders[i].Resources.m_DepthStencil),
                std::move(Builders[i].Resources.m_ResourcesCreated),
                std::move(ResourceToDestroy[i]),
                std::move(Builders[i].Resources.m_ResourceStates));
        }

        return Dependencies;
    }

    //

    IRenderPass& RenderGraphBuilder::AppendPass(
        std::unique_ptr<IRenderPass> Pass)
    {
        return *m_Passes.emplace_back(std::move(Pass));
    }

    RenderGraphBuilder::BuilderInfo::BuilderInfo(
        RenderGraphStorage&  Storage,
        const ShaderMapType& LoadedShaders) :
        Resources(Storage),
        RootSignatures(Storage),
        Shaders(Storage),
        PipelineStates(Storage, LoadedShaders)
    {
    }
} // namespace Renderer::RG
