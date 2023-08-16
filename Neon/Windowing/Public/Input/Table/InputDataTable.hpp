#pragma once

#include <Core/String.hpp>

#include <Input/Table/InputActionTable.hpp>
#include <Input/Table/InputAxisTable.hpp>
#include <Input/Table/InputMouseTable.hpp>
#include <Input/Table/InputMouseWheelTable.hpp>

#include <unordered_map>
#include <list>

namespace Neon::Windowing
{
    class WindowApp;
} // namespace Neon::Windowing

namespace Neon::Input
{
    class InputDataTable;

    class InputEventQueue
    {
        template<typename _Ty>
        using MemoryPoolType = std::list<_Ty>;

    public:
        /// <summary>
        /// Queue input data.
        /// </summary>
        template<typename _Ty, typename... _Args>
        void QueueData(
            _Args&&... Args)
        {
            GetPool<_Ty>().emplace_back(std::forward<_Args>(Args)...);
        }

        /// <summary>
        /// Dispatch input data.
        /// </summary>
        void Dispatch(
            InputDataTable* DataTable);

        /// <summary>
        /// Clear the input data.
        /// </summary>
        void ClearState();

    private:
        template<typename _Ty>
        [[nodiscard]] auto& GetPool() noexcept
        {
            if constexpr (std::is_same_v<_Ty, InputActionDataEvent>)
                return m_ActionPool;
            else if constexpr (std::is_same_v<_Ty, InputAxisDataEvent>)
                return m_AxisPool;
            else if constexpr (std::is_same_v<_Ty, InputMouseDataEvent>)
                return m_MousePool;
            else if constexpr (std::is_same_v<_Ty, InputMouseWheelDataEvent>)
                return m_MouseWheelPool;
        }

    private:
        MemoryPoolType<InputActionDataEvent>     m_ActionPool;
        MemoryPoolType<InputAxisDataEvent>       m_AxisPool;
        MemoryPoolType<InputMouseDataEvent>      m_MousePool;
        MemoryPoolType<InputMouseWheelDataEvent> m_MouseWheelPool;
    };

    /// <summary>
    /// An input data table is a collection of input tables.
    /// Each input table is a collection of input data that the user can hook into.
    ///
    /// For exemple, a user can create an input table for the action "Jump".
    /// This table will contain all the input data
    /// </summary>
    class InputDataTable
    {
        friend class Windowing::WindowApp;
        friend class InputEventQueue;

        using InputActionMap     = std::unordered_map<StringU8, Ptr<InputActionTable>>;
        using InputAxisMap       = std::unordered_map<StringU8, Ptr<InputAxisTable>>;
        using InputMouseMap      = std::unordered_map<StringU8, Ptr<InputMouseTable>>;
        using InputMouseWheelMap = std::unordered_map<StringU8, Ptr<InputMouseWheelTable>>;

    private:
        /// <summary>
        /// Insert a key press/tick/release event.
        /// </summary>
        void PushKey(
            int Key, int ScanCode, int Action, int Mods);

        /// <summary>
        /// Insert a mouse press/tick/release event.
        /// </summary>
        void PushMouseInput(
            int Key, int Action, int Mods);

        /// <summary>
        /// Insert a mouse move event.
        /// </summary>
        void PushMouseMove(
            double X, double Y);

        /// <summary>
        /// Reset the input state.
        /// </summary>
        void ClearState();

    public:
        /// <summary>
        /// Process queued inputs.
        /// </summary>
        void ProcessInputs();

    public:
        /// <summary>
        /// Get or create an action table.
        /// An action table is a collection of keyboard and gamepad inputs.
        /// </summary>
        Ptr<InputActionTable> LoadActionTable(
            const StringU8& ActionName);

        /// <summary>
        /// Get an action table.
        /// An action table is a collection of keyboard and gamepad inputs.
        /// </summary>
        Ptr<InputActionTable> GetActionTable(
            const StringU8& ActionName) const;

        /// <summary>
        /// Remove an action table.
        /// </summary>
        void RemoveActionTable(
            const StringU8& ActionName);

        //

        /// <summary>
        /// Get or create an axis table.
        /// An axis table is a collection of keyboard and gamepad press/hold/release inputs.
        /// </summary>
        Ptr<InputAxisTable> LoadAxisTable(
            const StringU8& AxisName);

        /// <summary>
        /// Get an axis table.
        /// An axis table is a collection of keyboard and gamepad press/hold/release inputs.
        /// </summary>
        Ptr<InputAxisTable> GetAxisTable(
            const StringU8& ActionName) const;

        /// <summary>
        /// Remove an axis table.
        /// </summary>
        void RemoveAxisTable(
            const StringU8& ActionName);

        //

        /// <summary>
        /// Get or create an mouse table.
        /// A mouse table is a collection of mouse inputs.
        /// </summary>
        Ptr<InputMouseTable> LoadMouseTable(
            const StringU8& MouseName);

        /// <summary>
        /// Get an mouse table.
        /// A mouse table is a collection of mouse inputs.
        /// </summary>
        Ptr<InputMouseTable> GetMouseTable(
            const StringU8& MouseName) const;

        /// <summary>
        /// Remove an mouse table.
        /// </summary>
        void RemoveMouseTable(
            const StringU8& MouseName);

        //

        /// <summary>
        /// Get or create an mouse wheel table.
        /// A mouse wheel table is a collection of mouse wheel inputs.
        /// </summary>
        Ptr<InputMouseWheelTable> LoadMouseWheelTable(
            const StringU8& MouseWheelName);

        /// <summary>
        /// Get an mouse wheel table.
        /// A mouse wheel table is a collection of mouse wheel inputs.
        /// </summary>
        Ptr<InputMouseWheelTable> GetMouseWheelTable(
            const StringU8& MouseWheelName) const;

        /// <summary>
        /// Remove an mouse wheel table.
        /// </summary>
        void RemoveMouseWheelTable(
            const StringU8& MouseWheelName);

    protected:
        InputActionMap     m_InputActionMap;
        InputAxisMap       m_InputAxisMap;
        InputMouseMap      m_InputMouseMap;
        InputMouseWheelMap m_InputMouseWheelMap;

        InputEventQueue m_EventQueue;
    };
} // namespace Neon::Input
