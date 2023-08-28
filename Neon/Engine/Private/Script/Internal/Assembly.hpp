#pragma once

#include <istream>

typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage    MonoImage;

namespace Neon::Scripting::CS
{
    class Assembly
    {
    public:
        Assembly(
            const char*   Name,
            std::istream& Stream);

        /// <summary>
        /// Gets the assembly.
        /// </summary>
        MonoAssembly* GetAssembly() const;

        /// <summary>
        /// Gets the image.
        /// </summary>
        MonoImage* GetImage() const;

    private:
        MonoAssembly* m_Assembly;
        MonoImage*    m_Image;
    };
} // namespace Neon::Scripting::CS