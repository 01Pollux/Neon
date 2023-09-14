#include <EnginePCH.hpp>
#include <Asset/Handlers/TextFile.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace Neon::Asset
{
    TextFileAsset::TextFileAsset(
        String        Text,
        const Handle& AssetGuid,
        StringU8      Path) :
        IAsset(AssetGuid, std::move(Path)),
        m_Text(std::move(Text))
    {
    }

    TextFileAsset::TextFileAsset(
        StringU8      Text,
        const Handle& AssetGuid,
        StringU8      Path) :
        IAsset(AssetGuid, std::move(Path)),
        m_Text(std::move(Text))
    {
    }

    const StringU8& TextFileAsset::AsUtf8() const
    {
        if (!std::holds_alternative<StringU8>(m_Text))
        {
            m_Text = StringUtils::Transform<StringU8>(std::get<String>(m_Text));
        }
        return std::get<StringU8>(m_Text);
    }

    const String& TextFileAsset::AsUtf16() const
    {
        if (!std::holds_alternative<String>(m_Text))
        {
            m_Text = StringUtils::Transform<String>(std::get<StringU8>(m_Text));
        }
        return std::get<String>(m_Text);
    }

    void TextFileAsset::SetText(
        const StringU8& Text)
    {
        MarkDirty();
        m_Text = StringUtils::Transform<StringU8>(Text);
    }

    void TextFileAsset::SetText(
        const String& Text)
    {
        MarkDirty();
        m_Text = StringUtils::Transform<String>(Text);
    }

    //

    bool TextFileAsset::Handler::CanHandle(
        const Ptr<IAsset>& Resource)
    {
        return dynamic_cast<TextFileAsset*>(Resource.get());
    }

    Ptr<IAsset> TextFileAsset::Handler::Load(
        std::istream&                  Stream,
        const Asset::DependencyReader& DepReader,
        const Handle&                  AssetGuid,
        StringU8                       Path,
        const AssetMetaData&           LoaderData)
    {
        StringU8 Text(std::istream_iterator<char>(Stream), {});

        return std::make_shared<TextFileAsset>(std::move(Text), AssetGuid, std::move(Path));
    }

    void TextFileAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        Stream << static_cast<const TextFileAsset*>(Asset.get())->AsUtf8();
    }
} // namespace Neon::Asset
