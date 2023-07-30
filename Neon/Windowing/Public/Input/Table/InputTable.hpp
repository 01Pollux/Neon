#pragma once

#include <Core/Neon.hpp>
#include <Input/Data/InputData.hpp>
#include <list>

namespace Neon::Input
{
    template<typename _Ty>
    class IInputTable
    {
        using InputDataList = std::vector<Ptr<_Ty>>;

    public:
        IInputTable() = default;

        NEON_CLASS_NO_COPYMOVE(IInputTable);

        virtual ~IInputTable() = default;

        /// <summary>
        /// Enable the input table.
        /// </summary>
        void Enable()
        {
            m_IsActive = true;
        }

        /// <summary>
        /// Disable the input table.
        /// </summary>
        void Disable()
        {
            m_IsActive = true;
        }

        /// <summary>
        /// Check if the input table is enabled.
        /// </summary>
        /// <returns></returns>
        bool IsEnabled() const noexcept
        {
            return m_IsActive;
        }

    protected:
        InputDataList m_InputDatas;
        bool          m_IsActive = false;
    };
} // namespace Neon::Input
