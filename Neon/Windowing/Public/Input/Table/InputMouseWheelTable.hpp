#pragma once

#include <Input/Table/InputTable.hpp>
#include <Input/Data/InputMouseWheel.hpp>

namespace Neon::Input
{
    class IInputMouseWheelTable : public IInputTable<InputMouseWheel>
    {
    public:
        /// <summary>
        /// Create a new input table.
        /// </summary>
        [[nodiscard]] static Ptr<IInputMouseWheelTable> Create();

        /// <summary>
        /// Add input data to the table.
        /// </summary>
        [[nodiscard]] const Ptr<InputMouseWheel>& AddMouseWheel()
        {
            return m_InputDatas.emplace_back(std::make_shared<InputMouseWheel>());
        }

        /// <summary>
        /// Remove input data from the table.
        /// </summary>
        void RemoveMouseWheel(
            const Ptr<InputMouseWheel>& MouseWheel)
        {
            std::erase_if(m_InputDatas, [MouseWheel](auto& Data)
                          { return Data == MouseWheel; });
        }

        /// <summary>
        /// Get mouse wheel inputs in the table.
        /// </summary>
        [[nodiscard]] auto& GetMouseWheels() const noexcept
        {
            return m_InputDatas;
        }
    };
} // namespace Neon::Input
