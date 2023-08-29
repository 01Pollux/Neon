#include <EnginePCH.hpp>
#include <Private/Script/Internal/Engine.hpp>
#include <Script/Internal/Object.hpp>
#include <Script/Internal/Utils.hpp>

namespace Neon::Scripting::CS
{
    const void* ObjectMarshaller<StringU8>::ToPtr(
        const StringU8& Obj)
    {
        return Utils::ToMonoString(Obj);
    }

    StringU8 ObjectMarshaller<StringU8>::FromPtr(
        MonoObject* Obj)
    {
        return Utils::FromMonoString(std::bit_cast<MonoString*>(Obj));
    }

    //

    Object::Object(
        MonoObject* Obj) :
        m_Object(Obj)
    {
    }

    MonoObject* Object::GetObject() const
    {
        return m_Object;
    }

    Object::operator bool() const
    {
        return m_Object;
    }

    MonoObject* Object::GetField(
        const char* Name,
        bool        IsProperty) const
    {
        MonoObject* Ret = nullptr;

        auto Cls = mono_object_get_class(m_Object);
        if (IsProperty)
        {
            auto Prop = mono_class_get_property_from_name(Cls, Name);
            Ret       = mono_property_get_value(Prop, m_Object, nullptr, nullptr);
        }
        else
        {
            auto Field = mono_class_get_field_from_name(Cls, Name);
            Ret        = mono_field_get_value_object(ScriptContext::Get()->CurrentDomain, Field, m_Object);
        }

        return Ret;
    }

    void Object::SetField(
        const char* Name,
        MonoObject* Value,
        bool        IsProperty)
    {
        auto Cls = mono_object_get_class(m_Object);
        if (IsProperty)
        {
            void* Params[] = { Value };
            auto  Prop     = mono_class_get_property_from_name(Cls, Name);
            mono_property_set_value(Prop, m_Object, Params, nullptr);
        }
        else
        {
            auto Cls   = mono_object_get_class(m_Object);
            auto Field = mono_class_get_field_from_name(Cls, Name);
            mono_field_set_value(m_Object, Field, Value);
        }
    }
} // namespace Neon::Scripting::CS