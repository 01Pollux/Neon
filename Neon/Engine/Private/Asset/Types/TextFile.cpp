#include <EnginePCH.hpp>
#include <Asset/Handlers/TextFile.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace Neon::Asset
{
    TextFileAsset::TextFileAsset(
        StringU8      Text,
        const Handle& AssetGuid,
        StringU8      Path) :
        IAsset(AssetGuid, std::move(Path)),
        m_Text(std::move(Text))
    {
    }

    const StringU8& TextFileAsset::Get() const
    {
        return m_Text;
    }

    void TextFileAsset::Set(
        const StringU8& Text)
    {
        MarkDirty();
        m_Text = Text;
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
        std::stringstream Text;
        Text << Stream.rdbuf();
        return std::make_shared<TextFileAsset>(Text.str(), AssetGuid, std::move(Path));
    }

    void TextFileAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        Stream << static_cast<const TextFileAsset*>(Asset.get())->Get();
    }
} // namespace Neon::Asset
