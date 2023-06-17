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
        /// Register an interface to the container with the same type
        /// </summary>
        template<typename _Ty, typename... _Args>
        Ptr<_Ty> RegisterInterface(
            _Args&&... Args)
        {
            return RegisterInterfaceOnce<_Ty, _Ty, _Args...>(std::forward<_Args>(Args)...);
        }

        /// <summary>
        /// Register an interface to the container or overwrite if already exists
        /// </summary>
        template<typename _Interface, typename _Ty, typename... _Args>
        Ptr<_Interface> OverwriteInterface(
            _Args&&... Args)
        {
            std::type_index TypeId(typeid(_Interface));
            return m_Interfaces[TypeId] = std::make_shared<_Ty>(std::forward<_Args>(Args)...);
        }

        /// <summary>
        /// Register an interface to the container or overwrite if already exists with the same type
        /// </summary>
        template<typename _Ty, typename... _Args>
        Ptr<_Ty> OverwriteInterface(
            _Args&&... Args)
        {
            return OverwriteInterface<_Ty, _Ty, _Args...>(std::forward<_Args>(Args)...);
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

    private:
        std::map<std::type_index, std::any> m_Interfaces;
    };
} // namespace Neon::Runtime