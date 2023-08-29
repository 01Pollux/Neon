#pragma once

#include <Private/Script/Internal/Object.hpp>

typedef struct _MonoClass  MonoClass;
typedef struct _MonoMethod MonoMethod;

namespace Neon::Scripting::CS
{
    class Class
    {
    public:
        Class(
            MonoClass* Cls);

        /// <summary>
        /// Gets the assembly.
        /// </summary>
        [[nodiscard]] MonoClass* GetClass() const;

        /// <summary>
        /// Construct a new object with the specified constructor.
        /// </summary>
        [[nodiscard]] Object New(
            const char*  MethodName,
            const void** Parameters,
            size_t       ParameterCount) const;

        /// <summary>
        /// Construct a new object with default constructor.
        /// </summary>
        [[nodiscard]] Object New() const;

        /// <summary>
        /// Gets the method with the specified name.
        /// </summary>
        [[nodiscard]] MonoMethod* GetMethod(
            const StringU8& Name) const;

        /// <summary>
        /// Gets the method with the specified name.
        /// </summary>
        [[nodiscard]] MonoMethod* GetMethod(
            size_t NameHash) const;

    private:
        /// <summary>
        /// Fetches all the methods in the class.
        /// </summary>
        void FetchMethods();

    private:
        MonoClass* m_Class;

        std::unordered_map<size_t, MonoMethod*> m_Methods;
    };
} // namespace Neon::Scripting::CS