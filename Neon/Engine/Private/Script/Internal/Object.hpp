#pragma once

typedef struct _MonoObject MonoObject;

namespace Neon::Scripting::CS
{
    class Object
    {
    public:
        Object(
            MonoObject* Obj);

        /// <summary>
        /// Gets the assembly.
        /// </summary>
        [[nodiscard]] MonoObject* GetObject() const;

        /// <summary>
        /// Checks if the object is valid (not null).
        /// </summary>
        operator bool() const;

    private:
        MonoObject* m_Object;
    };
} // namespace Neon::Scripting::CS