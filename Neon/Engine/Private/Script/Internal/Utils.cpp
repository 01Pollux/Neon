#include <EnginePCH.hpp>
#include <Private/Script/Internal/Engine.hpp>
#include <Script/Internal/Utils.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scripting::CS::Utils
{
    MonoObject* BoxValuePtr(
        MonoClass* Cls,
        void*      Ptr)
    {
        return mono_value_box(ScriptContext::Get()->CurrentDomain, Cls, Ptr);
    }

    void HandleException(
        MonoObject* Exception)
    {
        if (Exception) [[unlikely]]
        {
            MonoClass* Cls  = mono_object_get_class(Exception);
            MonoType*  Type = mono_class_get_type(Cls);

            auto GetExceptionString = [Exception, Cls](
                                          const char* Name) -> StringU8
            {
                MonoProperty* Property = mono_class_get_property_from_name(Cls, Name);

                if (Property == nullptr)
                    return "";

                MonoMethod* Getter = mono_property_get_get_method(Property);
                if (Getter == nullptr)
                    return "";

                return FromMonoString(std::bit_cast<MonoString*>(mono_runtime_invoke(Getter, Exception, NULL, NULL)));
            };

            NEON_ERROR_TAG("Script", "{}: {}. Source: {}, Stack trace: {}",
                           mono_type_get_name(Type),
                           GetExceptionString("Message"),
                           GetExceptionString("Source"),
                           GetExceptionString("StackTrace"));
        }
    }

    bool CheckMonoError(
        MonoError* Error)
    {
        if (!mono_error_ok(Error)) [[unlikely]]
        {
            auto Code    = mono_error_get_error_code(Error);
            auto Message = mono_error_get_message(Error);

            NEON_ERROR_TAG("Script", "Mono Error: {} ({})", Message, Code);
            mono_error_cleanup(Error);
            return true;
        }
        return false;
    }

    //

    StringU8 GetClassName(
        MonoClass* Class)
    {
        const char* ClassName = mono_class_get_name(Class);
        if (!ClassName || !ClassName[0])
            return "Unknown Class";

        StringU8 Str;

        MonoClass* NestingType = mono_class_get_nesting_type(Class);
        if (NestingType)
        {
            Str = GetClassName(NestingType) + "." + ClassName;
        }
        else
        {
            const char* Namespace = mono_class_get_namespace(Class);
            if (Namespace)
            {
                Str = StringU8(Namespace) + ".";
            }
            Str += ClassName;
        }

        MonoType* classType = mono_class_get_type(Class);
        if (mono_type_get_type(classType) == MONO_TYPE_SZARRAY || mono_type_get_type(classType) == MONO_TYPE_ARRAY)
        {
            Str = Str.substr(0, Str.length() - 2);
        }

        return Str;
    }

    MonoString* GetEmptyMonoString()
    {
        return mono_string_empty(ScriptContext::Get()->CurrentDomain);
    }

    MonoString* ToMonoString(
        const StringU8& Str)
    {
        return mono_string_new(ScriptContext::Get()->CurrentDomain, Str.c_str());
    }

    StringU8 FromMonoString(
        MonoString* Str)
    {
        int      StrLen = Str ? mono_string_length(Str) : 0;
        StringU8 Result(StrLen, '\0');

        if (StrLen)
        {
            MonoError Err;
            char*     CStr = mono_string_to_utf8_checked(Str, &Err);
            if (!CheckMonoError(&Err))
            {
                Result.assign(CStr);
                mono_free(CStr);
            }
        }
        return Result;
    }
} // namespace Neon::Scripting::CS::Utils