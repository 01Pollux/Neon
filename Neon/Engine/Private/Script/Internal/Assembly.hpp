#pragma once

#include <istream>
#include <Private/Script/Internal/Class.hpp>

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
        [[nodiscard]] MonoAssembly* GetAssembly() const;

        /// <summary>
        /// Gets the image.
        /// </summary>
        [[nodiscard]] MonoImage* GetImage() const;

        /// <summary>
        /// Gets the class with the specified name.
        /// </summary>
        [[nodiscard]] const Class* GetClass(
            const StringU8& Name) const;

        /// <summary>
        /// Gets the class with the specified name.
        /// </summary>
        [[nodiscard]] const Class* GetClass(
            size_t NameHash) const;

    private:
        /// <summary>
        /// Fetches all classes from the assembly.
        /// </summary>
        void FetchClasses();

    private:
        MonoAssembly* m_Assembly;
        MonoImage*    m_Image;

        std::unordered_map<size_t, Class> m_Classes;
    };
} // namespace Neon::Scripting::CS