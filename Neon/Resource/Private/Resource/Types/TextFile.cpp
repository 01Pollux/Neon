#include <ResourcePCH.hpp>
#include <Resource/Types/TextFile.hpp>

namespace Neon::Asset
{
    TextFileAsset::TextFileAsset(
        String Text) :
        m_Utf16Text(std::move(Text))
    {
    }

    const StringU8& TextFileAsset::AsUtf8() const
    {
        if (!m_Utf8Text)
        {
            m_Utf8Text = StringUtils::StringTransform<StringU8>(m_Utf16Text);
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
        m_Utf16Text = StringUtils::StringTransform<String>(Text);
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

    size_t TextFileAsset::Handler::QuerySize(
        const Ptr<IAssetResource>& Resource)
    {
        auto TextFile = static_cast<TextFileAsset*>(Resource.get());
        return (TextFile->AsUtf16().size() + 1) * sizeof(wchar_t);
    }

    Ptr<IAssetResource> TextFileAsset::Handler::Load(
        std::istream& Stream,
        size_t        DataSize)
    {
        String Str(DataSize, '\0');
        Stream.read(std::bit_cast<char*>(Str.data()), DataSize);
        return std::make_shared<TextFileAsset>(std::move(Str));
    }

    void TextFileAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        std::ostream&              Stream,
        size_t                     DataSize)
    {
        auto TextFile = static_cast<TextFileAsset*>(Resource.get());
        auto SrcData  = std::bit_cast<char*>(TextFile->AsUtf16().data());
        Stream.write(SrcData, DataSize);
    }
} // namespace Neon::Asset
