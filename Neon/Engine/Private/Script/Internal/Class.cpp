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
        const void** Parameters,
        size_t       ParameterCount) const
    {
        MonoObject* Obj = mono_object_new(ScriptContext::Get()->CurrentDomain, m_Class);

        MonoMethod* Ctor = nullptr;
        for (auto& Cur : GetMethods(".ctor"))
        {
            if (Cur.ParameterCount == ParameterCount)
            {
                Ctor = Cur.Method;
                break;
            }
        }

        Invoke(Ctor, Obj, Parameters, ParameterCount);
        return Obj;
    }

    Object Class::New() const
    {
        MonoObject* Obj = mono_object_new(ScriptContext::Get()->CurrentDomain, m_Class);
        mono_runtime_object_init(Obj);
        return Obj;
    }

    Asio::CoGenerator<MethodMetadata> Class::GetMethods(
        const StringU8& Name) const
    {
        return GetMethods(StringUtils::Hash(Name));
    }

    Asio::CoGenerator<MethodMetadata> Class::GetMethods(
        size_t NameHash) const
    {
        auto Cls = this;
        do
        {
            auto Iter = Cls->m_Methods.find(NameHash);
            for (; Iter != Cls->m_Methods.end() && Iter->first == NameHash; ++Iter)
            {
                co_yield static_cast<MethodMetadata>(Iter->second);
            }
        } while (Cls = Cls->GetParent());
    }

    //

    void Class::FetchMethods()
    {
        void* Iter = nullptr;
        while (MonoMethod* Method = mono_class_get_methods(m_Class, &Iter))
        {
            auto Signature = mono_method_signature(Method);

            m_Methods.emplace(
                StringUtils::Hash(mono_method_get_name(Method)),
                MethodMetadata{
                    .Method         = Method,
                    .ParameterCount = mono_signature_get_param_count(Signature) });
        }
    }
} // namespace Neon::Scripting::CS