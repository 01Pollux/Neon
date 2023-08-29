#include <EnginePCH.hpp>
#include <Private/Script/Internal/Engine.hpp>
#include <Script/Internal/Class.hpp>
#include <Script/Internal/Utils.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scripting::CS
{
    void* ObjectInvokerArg<StringU8>::ToPtr(
        StringU8& Obj)
    {
        return Utils::ToMonoString(Obj);
    }

    StringU8 ObjectInvokerArg<StringU8>::FromPtr(
        MonoObject* Obj)
    {
        return Utils::FromMonoString(std::bit_cast<MonoString*>(Obj));
    }

    //

    Class::Class(
        MonoClass* Cls) :
        m_Class(Cls)
    {
        FetchMethods();
    }

    const Class* Class::GetParent() const
    {
        return m_BaseClass;
    }

    MonoClass* Class::GetClass() const
    {
        return m_Class;
    }

    MonoObject* Class::Invoke(
        MonoMethod*  Method,
        MonoObject*  Obj,
        const void** Parameters,
        size_t       ParameterCount) const
    {
        MonoObject* Exception;
        auto        Object = mono_runtime_invoke(Method, Obj, const_cast<void**>(Parameters), &Exception);
        Utils::HandleException(Exception);
        return Object;
    }

    Object Class::New(
        std::span<const char*> ParameterTypes,
        const void**           Parameters,
        size_t                 ParameterCount) const
    {
        MonoObject* Obj  = mono_object_new(ScriptContext::Get()->CurrentDomain, m_Class);
        MonoMethod* Ctor = FindMethod(".ctor", ParameterTypes);
        Invoke(Ctor, Obj, Parameters, ParameterCount);
        return Obj;
    }

    Object Class::New() const
    {
        MonoObject* Obj = mono_object_new(ScriptContext::Get()->CurrentDomain, m_Class);
        mono_runtime_object_init(Obj);
        return Obj;
    }

    Asio::CoGenerator<const MethodMetadata&> Class::GetMethods(
        const char* Name) const
    {
        return GetMethods(StringUtils::Hash(Name));
    }

    Asio::CoGenerator<const MethodMetadata&> Class::GetMethods(
        size_t NameHash) const
    {
        auto Cls = this;
        do
        {
            auto Iter = Cls->m_Methods.find(NameHash);
            for (; Iter != Cls->m_Methods.end() && Iter->first == NameHash; ++Iter)
            {
                co_yield Iter->second;
            }
        } while (Cls = Cls->GetParent());
    }

    MonoMethod* Class::FindMethod(
        const char*            Name,
        std::span<const char*> ParameterTypes) const
    {
        return FindMethod(StringUtils::Hash(Name), ParameterTypes);
    }

    MonoMethod* Class::FindMethod(
        size_t                 NameHash,
        std::span<const char*> ParameterTypes) const
    {
        for (auto& Cur : GetMethods(NameHash))
        {
            if (Cur.Parameters.size() == ParameterTypes.size())
            {
                bool Matches = true;
                for (size_t i = 0; i < ParameterTypes.size(); ++i)
                {
                    if (Cur.Parameters[i] != ParameterTypes[i])
                    {
                        Matches = false;
                        break;
                    }
                }
                if (Matches)
                {
                    return Cur.Method;
                }
            }
        }
        return nullptr;
    }

    //

    void Class::FetchMethods()
    {
        void* Iter = nullptr;
        while (MonoMethod* Method = mono_class_get_methods(m_Class, &Iter))
        {
            auto   Signature   = mono_method_signature(Method);
            size_t ParamsCount = mono_signature_get_param_count(Signature);

            std::vector<StringU8> Parameters;
            Parameters.reserve(ParamsCount);

            void* ParamIter = nullptr;
            while (auto Param = mono_signature_get_params(Signature, &ParamIter))
            {
                auto TypeName = mono_type_get_name(Param);
                Parameters.emplace_back(TypeName);
                mono_free(TypeName);
            }

            m_Methods.emplace(
                StringUtils::Hash(mono_method_get_name(Method)),
                MethodMetadata{
                    .Method     = Method,
                    .Parameters = std::move(Parameters) });
        }
    }
} // namespace Neon::Scripting::CS