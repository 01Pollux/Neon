#pragma once

#include <Core/Neon.hpp>
#include <map>
#include <any>
#include <memory>
#include <typeindex>

namespace Neon::Runtime
{
    class InterfaceContainer
    {
    public:
        /// <summary>
        /// Register an interface to the container
        /// </summary>
        template<typename _Interface, typename _FnTy>
            requires std::is_invocable_r_v<Ptr<_Interface>, _FnTy>
        Ptr<_Interface> RegisterInterface(
            _FnTy&& InterfaceCreator)
        {
            Ptr<_Interface> Interface;
            std::type_index TypeId(typeid(_Interface));
            if (!ContainsInterface<_Interface>())
            {
                m_Interfaces[TypeId] = Interface = InterfaceCreator();
            }
            else
            {
                Interface = std::any_cast<Ptr<_Interface>>(m_Interfaces[TypeId]);
            }
            return Interface;
        }

        /// <summary>
        /// Register an interface to the container
        /// </summary>
        template<typename _Interface, typename _Ty, typename... _Args>
            requires std::is_base_of_v<_Interface, _Ty>
        Ptr<_Ty> RegisterInterface(
            _Args&&... Args)
        {
            return std::static_pointer_cast<_Ty>(RegisterInterface<_Interface>(
                [&]
                { return std::make_shared<_Ty>(std::forward<_Args>(Args)...); }));
        }

        /// <summary>
        /// Register an interface to the container or overwrite if already exists
        /// </summary>
        template<typename _Interface, typename _Ty, typename... _Args>
        Ptr<_Interface> OverwriteInterface(
            _Args&&... Args)
        {
            std::type_index TypeId(typeid(_Interface));
            Ptr<_Interface> Interface = std::make_shared<_Ty>(std::forward<_Args>(Args)...);
            m_Interfaces[TypeId]      = Interface;
            return Interface;
        }

        /// <summary>
        /// Unregister an interface from the container
        /// </summary>
        template<typename _Interface>
        void UnregisterInterface()
        {
            std::type_index TypeId(typeid(_Interface));
            m_Interfaces.erase(TypeId);
        }

        /// <summary>
        /// Check if the interface is registered
        /// </summary>
        template<typename _Interface>
        [[nodiscard]] bool ContainsInterface()
        {
            return m_Interfaces.contains(std::type_index(typeid(_Interface)));
        }

        /// <summary>
        /// Requires interface to be present
        /// </summary>
        template<typename _Interface>
        [[nodiscard]] void RequireInterface()
        {
            if (!ContainsInterface<_Interface>())
            {
                throw std::runtime_error("Interface doesn't exists");
            }
        }

        /// <summary>
        /// Query an interface from the container and return nullptr if not found
        /// </summary>
        template<typename _Interface>
        std::shared_ptr<_Interface> QueryInterface()
        {
            auto It = m_Interfaces.find(std::type_index(typeid(_Interface)));
            if (It != m_Interfaces.end())
            {
                return std::any_cast<std::shared_ptr<_Interface>>(It->second);
            }
            return nullptr;
        }

        /// <summary>
        /// Unregister all interfaces from the container
        /// </summary>
        void UnregisterAllInterfaces()
        {
            m_Interfaces.clear();
        }

    private:
        std::map<std::type_index, std::any> m_Interfaces;
    };
} // namespace Neon::Runtime