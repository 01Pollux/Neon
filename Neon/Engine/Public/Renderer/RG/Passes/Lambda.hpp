#pragma once

#include <Renderer/RG/Pass.hpp>
#include <functional>

namespace Neon::RG
{
    class GraphStorage;

    class LambdaPass : public IRenderPass
    {
    public:
        LambdaPass(
            PassQueueType QueueType) :
            IRenderPass(QueueType)
        {
        }

        void ResolveShaders(
            ShaderResolver& Resolver) override
        {
            if (m_ShaderResolver)
            {
                m_ShaderResolver(Resolver);
            }
        }

        void ResolveRootSignature(
            RootSignatureResolver& Resolver) override
        {
            if (m_RootSignatureResolver)
            {
                m_RootSignatureResolver(Resolver);
            }
        }

        void ResolvePipelineStates(
            PipelineStateResolver& Resolver) override
        {
            if (m_PipelineStateResolver)
            {
                m_PipelineStateResolver(Resolver);
            }
        }

        void ResolveResources(
            ResourceResolver& Resolver) override
        {
            if (m_ResourceResolver)
            {
                m_ResourceResolver(Resolver);
            }
        }

        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList) override
        {
            if (m_Dispatcher)
            {
                m_Dispatcher(Storage, CommandList);
            }
        }

    public:
        /// <summary>
        /// Set the shader resolver for the pass.
        /// </summary>
        template<typename _FnTy, typename... _Args>
        LambdaPass& SetShaderResolver(
            _FnTy&& Resolver,
            _Args&&... Args)
        {
            m_ShaderResolver = std ::bind_back(std::forward<_FnTy>(Resolver), std::forward<_Args>(Args)...);
            return *this;
        }

        /// <summary>
        /// Set the root signature resolver for the pass.
        /// </summary>
        template<typename _FnTy, typename... _Args>
        LambdaPass& SetRootSignatureResolver(
            _FnTy&& Resolver,
            _Args&&... Args)
        {
            m_RootSignatureResolver = std::bind_back(std::forward<_FnTy>(Resolver), std::forward<_Args>(Args)...);
            return *this;
        }

        /// <summary>
        /// Set the pipeline state resolver for the pass.
        /// </summary>
        template<typename _FnTy, typename... _Args>
        LambdaPass& SetPipelineStateResolver(
            _FnTy&& Resolver,
            _Args&&... Args)
        {
            m_PipelineStateResolver = std::bind_back(std::forward<_FnTy>(Resolver), std::forward<_Args>(Args)...);
            return *this;
        }

        /// <summary>
        /// Set the resource resolver for the pass.
        /// </summary>
        template<typename _FnTy, typename... _Args>
        LambdaPass& SetResourceResolver(
            _FnTy&& Resolver,
            _Args&&... Args)
        {
            m_ResourceResolver = std::bind_back(std::forward<_FnTy>(Resolver), std::forward<_Args>(Args)...);
            return *this;
        }

        /// <summary>
        /// Set the dispatcher for the pass.
        /// </summary>
        template<typename _FnTy, typename... _Args>
        LambdaPass& SetDispatcher(
            _FnTy&& Resolver,
            _Args&&... Args)
        {
            m_Dispatcher = std::bind_back(std::forward<_FnTy>(Resolver), std::forward<_Args>(Args)...);
            return *this;
        }

    private:
        std::function<void(ShaderResolver&)>
                                                    m_ShaderResolver;
        std::function<void(RootSignatureResolver&)> m_RootSignatureResolver;
        std::function<void(PipelineStateResolver&)> m_PipelineStateResolver;
        std::function<void(ResourceResolver&)>      m_ResourceResolver;

        std::function<void(const GraphStorage&, RHI::ICommandList*)> m_Dispatcher;
    };
} // namespace Neon::RG