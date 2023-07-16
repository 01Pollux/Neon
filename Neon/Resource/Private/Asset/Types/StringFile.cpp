#include <ResourcePCH.hpp>
#include <Asset/Types/StringFile.hpp>

namespace Neon::AAsset
{
    StringFileAsset::StringFileAsset(
        const StringU8&             Text,
        const Ptr<StringFileAsset>& Child,
        const Handle&               Handle) :
        IAsset(Handle),
        m_Text(Text),
        m_Child(Child)
    {
    }

    const StringU8& StringFileAsset::GetText() const
    {
        return m_Text;
    }

    void StringFileAsset::SetText(
        StringU8 Text)
    {
        m_Text = std::move(Text);
    }
} // namespace Neon::AAsset
