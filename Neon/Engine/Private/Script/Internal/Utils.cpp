#include <EnginePCH.hpp>
#include <Private/Script/Internal/Utils.hpp>
#include <Private/Script/Internal/Engine.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scripting::CS::Utils
{
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
            Str = GetClassName(NestingType) + "/" + ClassName;
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
        MonoError Err;
        char*     CStr    = mono_string_to_utf8_checked(Str, &Err);
        bool      Success = !CheckMonoError(&Err);
        StringU8  Result(Success ? CStr : "");
        if (Success)
        {
            mono_free(CStr);
        }
        return Result;
    }
} // namespace Neon::Scripting::CS::Utils