#include <WindowPCH.hpp>
#include <Window/Window.hpp>
#include <Input/Table.hpp>

namespace Neon::Windowing
{
    void IWindowApp::PushInputTable(
        const Ptr<Input::IInputDataTable>& InputTable)
    {
        m_InputTables.insert(InputTable);
    }

    void IWindowApp::PopInputTable(
        const Ptr<Input::IInputDataTable>& InputTable)
    {
        m_InputTables.erase(InputTable);
    }

    void IWindowApp::ProcessInputs()
    {
        for (const auto& InputTable : m_InputTables)
        {
            InputTable->ProcessInputs();
        }
    }
} // namespace Neon::Windowing