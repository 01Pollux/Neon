#pragma once

#include <Input/Table/InputTable.hpp>
#include <Input/Data/InputAxis.hpp>

namespace Neon::Input
{
    class IInputAxisTable : public IInputTable<IInputAxis>
    {
    public:
        /// <summary>
        /// Create a new input table.
        /// </summary>
        [[nodiscard]] static Ptr<IInputAxisTable> Create();

        /// <summary>
        /// Add input data to the table.
        /// </summary>
        [[nodiscard]] IInputAxis* AddMouse()
        {
            auto Action    = std::make_unique<IInputAxis>();
            auto ActionPtr = Action.get();
            m_InputDatas.emplace_back(std::move(Action));
            return ActionPtr;
        }

        /// <summary>
        /// Remove input data from the table.
        /// </summary>
        void RemoveMouse(
            IInputAxis* Action)
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
        [[nodiscard]] IInputAxis* GetMouse(
            size_t ActionIndex)
        {
            return m_InputDatas[ActionIndex].get();
        }
    };
} // namespace Neon::Input
