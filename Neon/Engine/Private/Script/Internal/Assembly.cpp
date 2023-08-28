#include <EnginePCH.hpp>
#include <Private/Script/Internal/Assembly.hpp>

#include <Mono/jit/jit.h>
#include <Mono/metadata/assembly.h>
#include <Mono/metadata/image.h>
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
    }

    MonoAssembly* Assembly::GetAssembly() const
    {
        return m_Assembly;
    }

    MonoImage* Assembly::GetImage() const
    {
        return m_Image;
    }
} // namespace Neon::Scripting::CS