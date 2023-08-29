#include <EnginePCH.hpp>
#include <Private/Script/Internal/Assembly.hpp>
#include <Private/Script/Internal/Utils.hpp>

#include <Mono/metadata/image.h>
#include <Mono/metadata/tokentype.h>
#include <vector>

#include <Log/Logger.hpp>

namespace Neon::Scripting::CS
{
    Assembly::Assembly(
        const char*   Name,
        std::istream& Stream)
    {
        std::vector<char> Buffer{ std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>() };

        MonoImageOpenStatus Status;

        m_Image = mono_image_open_from_data_full(Buffer.data(), uint32_t(Buffer.size()), true, &Status, false);
        NEON_VALIDATE(Status == MONO_IMAGE_OK, "Failed to open C# image: {}.", mono_image_strerror(Status));

        m_Assembly = mono_assembly_load_from_full(m_Image, Name, &Status, 0);
        NEON_VALIDATE(Status == MONO_IMAGE_OK, "Failed to open C# assembly: {}.", mono_image_strerror(Status));

        mono_image_close(m_Image);

        FetchClasses();
    }

    MonoAssembly* Assembly::GetAssembly() const
    {
        return m_Assembly;
    }

    MonoImage* Assembly::GetImage() const
    {
        return m_Image;
    }

    const Class* Assembly::GetClass(
        const StringU8& Name) const
    {
        return GetClass(StringUtils::Hash(Name));
    }

    const Class* Assembly::GetClass(
        size_t NameHash) const
    {
        auto Iter = m_Classes.find(NameHash);
        return Iter != m_Classes.end() ? &Iter->second : nullptr;
    }

    //

    void Assembly::FetchClasses()
    {
        auto    Table = mono_image_get_table_info(m_Image, MONO_TABLE_TYPEDEF);
        int32_t Rows  = mono_table_info_get_rows(Table);

        for (int i = 0; i < Rows; i++)
        {
            MonoClass* Class = mono_class_get(m_Image, (i + 1) | MONO_TOKEN_TYPE_DEF);
            m_Classes.emplace(StringUtils::Hash(Utils::GetClassName(Class)), Class);
        }
    }
} // namespace Neon::Scripting::CS