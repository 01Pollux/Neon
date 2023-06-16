#pragma once

#include <Renderer/RG/Pass.hpp>
#include <functional>

namespace Neon::RG
{
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
        template<std::invocable<ShaderResolver&> _FnTy>
        LambdaPass& SetShaderResolver(
            _FnTy&& Resolver)
        {
            m_ShaderResolver = std::move(Resolver);
            return *this;
        }

        /// <summary>
        /// Set the root signature resolver for the pass.
        /// </summary>
        template<std::invocable<RootSignatureResolver&> _FnTy>
        LambdaPass& SetRootSignatureResolver(
            _FnTy&& Resolver)
        {
            m_RootSignatureResolver = std::move(Resolver);
            return *this;
        }

        /// <summary>
        /// Set the pipeline state resolver for the pass.
        /// </summary>
        template<std::invocable<PipelineStateResolver&> _FnTy>
        LambdaPass& SetPipelineStateResolver(
            _FnTy&& Resolver)
        {
            m_PipelineStateResolver = std::move(Resolver);
            return *this;
        }

        /// <summary>
        /// Set the resource resolver for the pass.
        /// </summary>
        template<std::invocable<ResourceResolver&> _FnTy>
        LambdaPass& SetResourceResolver(
            _FnTy&& Resolver)
        {
            m_ResourceResolver = std::move(Resolver);
            return *this;
        }

        /// <summary>
        /// Set the dispatcher for the pass.
        /// </summary>
        template<std::invocable<const GraphStorage&, RHI::ICommandList*> _FnTy>
        LambdaPass& SetDispatcher(
            _FnTy&& Dispatcher)
        {
            m_Dispatcher = std::move(Dispatcher);
            return *this;
        }

    private:
        std::function<void(ShaderResolver&)>        m_ShaderResolver;
        std::function<void(RootSignatureResolver&)> m_RootSignatureResolver;
        std::function<void(PipelineStateResolver&)> m_PipelineStateResolver;
        std::function<void(ResourceResolver&)>      m_ResourceResolver;

        std::function<void(const GraphStorage&, RHI::ICommandList*)> m_Dispatcher;
    };
} // namespace Neon::RG