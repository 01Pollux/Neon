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
        /// Set the resource resolver for the pass.
        /// </summary>
        template<typename _FnTy, typename... _Args>
            requires std::invocable<_FnTy, _Args...>
        LambdaPass& SetResourceResolver(
            _FnTy Resolver,
            _Args... Args)
        {
            m_ResourceResolver = std::bind(std::placeholders::_1, std::move(Resolver), std::move(Args)...);
            return *this;
        }

        /// <summary>
        /// Set the dispatcher for the pass.
        /// </summary>
        template<typename _FnTy, typename... _Args>
            requires std::invocable<_FnTy, _Args...>
        LambdaPass& SetDispatcher(
            _FnTy Resolver,
            _Args... Args)
        {
            m_Dispatcher = std::bind(std::move(Resolver), std::placeholders::_1, std::placeholders::_2, std::move(Args)...);
            return *this;
        }

    private:
        std::move_only_function<void(ResourceResolver&)>                       m_ResourceResolver;
        std::move_only_function<void(const GraphStorage&, RHI::ICommandList*)> m_Dispatcher;
    };
} // namespace Neon::RG