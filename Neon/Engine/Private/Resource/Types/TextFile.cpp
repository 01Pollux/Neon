#include <EnginePCH.hpp>
#include <Resource/Types/TextFile.hpp>

namespace Neon::Asset
{
    TextFileAsset::TextFileAsset(
        String Text) :
        m_Utf16Text(std::move(Text))
    {
    }

    TextFileAsset::TextFileAsset(
        StringU8 Text) :
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
        m_Utf8Text  = Text;
        m_Utf16Text = StringUtils::Transform<String>(Text);
    }

    void TextFileAsset::SetText(
        const String& Text)
    {
        m_Utf8Text.reset();
        m_Utf16Text = Text;
    }

    //

    bool TextFileAsset::Handler::CanCastTo(
        const Ptr<IAssetResource>& Resource)
    {
        return dynamic_cast<TextFileAsset*>(Resource.get());
    }

    Ptr<IAssetResource> TextFileAsset::Handler::Load(
        IO::BinaryStreamReader Stream,
        size_t)
    {
        return std::make_shared<TextFileAsset>(Stream.Read<String>());
    }

    void TextFileAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        IO::BinaryStreamWriter     Stream)
    {
        auto TextFile = static_cast<TextFileAsset*>(Resource.get());
        Stream.Write(TextFile->AsUtf16());
    }
} // namespace Neon::Asset
