#pragma once

#include <Core/Neon.hpp>
#include <Input/Types.hpp>
#include <bitset>

namespace Neon::Input
{
    class InputSysKeyState
    {
        static constexpr size_t SysInputControlCounts = int(EKeyboardInput::_End_SysContrl) - int(EKeyboardInput::_Begin_SysControl) + 1;
        using SysKeyStateMask                         = std::bitset<SysInputControlCounts>;

    public:
        /// <summary>
        /// Get the state of a system control key
        /// </summary>
        [[nodiscard]] bool GetSysControlState(
            EKeyboardInput InputType) const
        {
            if (InputType >= EKeyboardInput::_Begin_SysControl && InputType <= EKeyboardInput::_End_SysContrl)
            {
                return m_KeysState.test((size_t(InputType) - int(EKeyboardInput::_Begin_SysControl)));
            }
            return false;
        }

        /// <summary>
        /// Set the state of a system control key
        /// </summary>
        void SetSysControlState(
            EKeyboardInput InputType,
            bool           State)
        {
            if (InputType >= EKeyboardInput::_Begin_SysControl && InputType <= EKeyboardInput::_End_SysContrl)
            {
                m_KeysState.set((size_t(InputType) - int(EKeyboardInput::_Begin_SysControl)), State);
            }
        }

        /// <summary>
        /// Compare two InputSysKeyState
        /// </summary>
        [[nodiscard]] bool operator==(
            const InputSysKeyState& Other) const noexcept
        {
            return m_KeysState == Other.m_KeysState;
        }

        /// <summary>
        /// Compare two InputSysKeyState
        /// </summary>
        [[nodiscard]] bool ContainsAll(
            const InputSysKeyState& Other) const noexcept
        {
            for (size_t i = 0; i < SysInputControlCounts; i++)
            {
                if (m_KeysState.test(i) && !Other.m_KeysState.test(i))
                {
                    return false;
                }
            }
            return true;
        }

        /// <summary>
        /// Check if the InputSysKeyState is empty (no keys pressed)
        /// </summary>
        bool IsEmpty() const noexcept
        {
            return m_KeysState.none();
        }

    private:
        SysKeyStateMask m_KeysState{};
    };
} // namespace Neon::Input
