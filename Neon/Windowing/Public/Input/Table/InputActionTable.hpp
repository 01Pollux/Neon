#pragma once

#include <Input/Table/InputTable.hpp>
#include <Input/Data/InputAction.hpp>

namespace Neon::Input
{
    class IInputActionTable : public IInputTable<InputAction>
    {
    public:
        /// <summary>
        /// Create a new input table.
        /// </summary>
        [[nodiscard]] static Ptr<IInputActionTable> Create();

        /// <summary>
        /// Add input data to the table.
        /// </summary>
        [[nodiscard]] Ptr<InputAction> AddAction()
        {
            return m_InputDatas.emplace_back(std::make_shared<InputAction>());
        }

        /// <summary>
        /// Remove input data from the table.
        /// </summary>
        void RemoveAction(
            const Ptr<InputAction>& Action)
        {
            std::erase_if(m_InputDatas, [&Action](auto& Data)
                          { return Data == Action; });
        }

        /// <summary>
        /// Get action inputs in the table.
        /// </summary>
        [[nodiscard]] auto& GetActions() const noexcept
        {
            return m_InputDatas;
        }
    };
} // namespace Neon::Input
