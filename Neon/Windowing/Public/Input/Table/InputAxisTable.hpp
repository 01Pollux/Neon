#pragma once

#include <Input/Table/InputTable.hpp>
#include <Input/Data/InputAxis.hpp>

namespace Neon::Input
{
    class InputAxisTable : public IInputTable<InputAxis>
    {
    public:
        /// <summary>
        /// Add input data to the table.
        /// </summary>
        [[nodiscard]] const Ptr<InputAxis>& AddAxis()
        {
            return m_InputDatas.emplace_back(std::make_shared<InputAxis>());
        }

        /// <summary>
        /// Remove input data from the table.
        /// </summary>
        void RemoveAxis(
            const Ptr<InputAxis>& Axis)
        {
            std::erase_if(m_InputDatas, [&Axis](auto& Data)
                          { return Data == Axis; });
        }

        /// <summary>
        /// Get axis inputs in the table.
        /// </summary>
        [[nodiscard]] auto& GetAxis() const noexcept
        {
            return m_InputDatas;
        }
    };
} // namespace Neon::Input
