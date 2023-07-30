#pragma once

#include <Input/Table/InputTable.hpp>
#include <Input/Data/InputMouseWheel.hpp>

namespace Neon::Input
{
    class IInputMouseWheelTable : public IInputTable<IInputMouseWheel>
    {
    public:
        /// <summary>
        /// Create a new input table.
        /// </summary>
        [[nodiscard]] static Ptr<IInputMouseWheelTable> Create();

        /// <summary>
        /// Add input data to the table.
        /// </summary>
        [[nodiscard]] IInputMouseWheel* AddMouse()
        {
            auto Action    = std::make_unique<IInputMouseWheel>();
            auto ActionPtr = Action.get();
            m_InputDatas.emplace_back(std::move(Action));
            return ActionPtr;
        }

        /// <summary>
        /// Remove input data from the table.
        /// </summary>
        void RemoveMouse(
            IInputMouseWheel* Action)
        {
            std::erase_if(m_InputDatas, [Action](auto& Data)
                          { return Data.get() == Action; });
        }

        /// <summary>
        /// Get the number of input data in the table.
        /// </summary>
        [[nodiscard]] size_t GetMouseCount() const
        {
            return m_InputDatas.size();
        }

        /// <summary>
        /// Get input data from the table.
        /// </summary>
        [[nodiscard]] IInputMouseWheel* GetMouse(
            size_t ActionIndex)
        {
            return m_InputDatas[ActionIndex].get();
        }
    };
} // namespace Neon::Input
