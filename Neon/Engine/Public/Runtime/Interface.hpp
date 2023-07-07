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
        template<typename _Interface, typename _Ty, typename... _Args>
        Ptr<_Interface> RegisterInterface(
            _Args&&... Args)
        {
            Ptr<_Interface> Interface;

            std::type_index TypeId(typeid(_Interface));
            if (!ContainsInterface<_Interface>())
            {
                m_Interfaces[TypeId] = Interface = std::make_shared<_Ty>(std::forward<_Args>(Args)...);
            }
            return Interface;
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