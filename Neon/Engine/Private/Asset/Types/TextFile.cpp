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
        m_Utf16Text(std::move(Text))
    {
    }

    TextFileAsset::TextFileAsset(
        StringU8      Text,
        const Handle& AssetGuid,
        StringU8      Path) :
        IAsset(AssetGuid, std::move(Path)),
        m_Utf16Text(StringUtils::Transform<String>(Text)),
        m_Utf8Text(std::move(Text))
    {
    }

    const StringU8& TextFileAsset::AsUtf8() const
    {
        if (!m_Utf8Text)
        {
            m_Utf8Text = StringUtils::Transform<StringU8>(m_Utf16Text);
        }
        return *m_Utf8Text;
    }

    const String& TextFileAsset::AsUtf16() const
    {
        return m_Utf16Text;
    }

    void TextFileAsset::SetText(
        const StringU8& Text)
    {
        MarkDirty();
        m_Utf8Text  = Text;
        m_Utf16Text = StringUtils::Transform<String>(Text);
    }

    void TextFileAsset::SetText(
        const String& Text)
    {
        MarkDirty();
        m_Utf8Text.reset();
        m_Utf16Text = Text;
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
        boost::archive::text_iarchive Archive(Stream, boost::archive::no_header | boost::archive::no_tracking);

        String Text;
        Archive >> Text;

        return std::make_shared<TextFileAsset>(std::move(Text), AssetGuid, std::move(Path));
    }

    void TextFileAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto TextFile = static_cast<TextFileAsset*>(Asset.get());

        boost::archive::text_oarchive Archive(Stream, boost::archive::no_header | boost::archive::no_tracking);
        Archive << TextFile->AsUtf16();
    }
} // namespace Neon::Asset
