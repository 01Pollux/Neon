#pragma once

#include <Input/Table/InputTable.hpp>
#include <Input/Data/InputMouse.hpp>

namespace Neon::Input
{
    class InputMouseTable : public IInputTable<InputMouse>
    {
    public:
        /// <summary>
        /// Add input data to the table.
        /// </summary>
        [[nodiscard]] const Ptr<InputMouse>& AddMouse()
        {
            return m_InputDatas.emplace_back(std::make_shared<InputMouse>());
        }

        /// <summary>
        /// Remove input data from the table.
        /// </summary>
        void RemoveMouse(
            const Ptr<InputMouse>& Mouse)
        {
            std::erase_if(m_InputDatas, [&Mouse](auto& Data)
                          { return Data == Mouse; });
        }

        /// <summary>
        /// Get mouse inputs in the table.
        /// </summary>
        [[nodiscard]] auto& GetMouses() const noexcept
        {
            return m_InputDatas;
        }
    };
} // namespace Neon::Input
