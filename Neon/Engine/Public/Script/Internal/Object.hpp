#pragma once

#include <Mono/metadata/object.h>

namespace Neon::Scripting::CS
{
    template<typename _Ty>
    struct ObjectMarshaller
    {
        static void* ToPtr(
            const _Ty& Obj)
        {
            return const_cast<void*>(std::bit_cast<const void*>(std::addressof(Obj)));
        }

        static _Ty FromPtr(
            MonoObject* Obj)
        {
            return *std::bit_cast<_Ty*>(mono_object_unbox(Obj));
        }
    };

    //

    template<>
    struct ObjectMarshaller<StringU8>
    {
        static const void* ToPtr(
            const StringU8& Obj);

        static StringU8 FromPtr(
            MonoObject* Obj);
    };

    //

    template<>
    struct ObjectMarshaller<MonoObject*>
    {
        static const void* ToPtr(
            MonoObject* Obj)
        {
            return Obj;
        }

        static MonoObject* FromPtr(
            MonoObject* Obj)
        {
            return Obj;
        }
    };

    //

    class Object
    {
    public:
        Object(
            MonoObject* Obj = nullptr);

        /// <summary>
        /// Gets the assembly.
        /// </summary>
        [[nodiscard]] MonoObject* GetObject() const;

        /// <summary>
        /// Checks if the object is valid (not null).
        /// </summary>
        operator bool() const;

    public:
        /// <summary>
        /// Gets the field with the specified name.
        /// </summary>
        MonoObject* GetField(
            const char* Name,
            bool        IsProperty = false) const;

        /// <summary>
        /// Sets the field with the specified name.
        /// </summary>
        void SetField(
            const char* Name,
            MonoObject* Value,
            bool        IsProperty = false);

    public:
        /// <summary>
        /// Gets the field with the specified name.
        /// </summary>
        template<typename _Ty>
        _Ty GetFieldAs(
            const char* Name,
            bool        IsProperty = false) const
        {
            return ObjectMarshaller<_Ty>::FromPtr(GetField(Name, IsProperty));
        }

        /// <summary>
        /// Sets the field with the specified name.
        /// </summary>
        template<typename _Ty>
        void SetFieldAs(
            const char* Name,
            const _Ty&  Value,
            bool        IsProperty = false)
        {
            SetField(Name, std::bit_cast<MonoObject*>(ObjectMarshaller<_Ty>::ToPtr(Value)), IsProperty);
        }

    private:
        MonoObject* m_Object;
    };
} // namespace Neon::Scripting::CS