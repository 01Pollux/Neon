#include <GraphicsPCH.hpp>

#include <RHI/Material/VarBuilder.hpp>
#include <RHI/Resource/Common.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    void MaterialVarBuilder::SetSharedData(
        const Structured::LayoutBuilder& Builder)
    {
        // Shared data is stored in constant buffer and must be have 16 bytes alignement
        NEON_ASSERT(Builder.GetAlignement() == 16);
        m_SharedLayout.Layout = Builder.Cook(true);
    }

    void MaterialVarBuilder::SetLocalData(
        const Structured::LayoutBuilder& Builder)
    {
        // Shared data is stored in structured buffer and must be have 16 bytes alignement
        NEON_ASSERT(Builder.GetAlignement() == 4);
        m_SharedLayout.Layout = Builder.Cook(true);
    }
} // namespace Neon::RHI