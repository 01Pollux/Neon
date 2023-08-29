#include <EnginePCH.hpp>
#include <Private/Script/Internal/Class.hpp>
#include <Private/Script/Internal/Engine.hpp>
#include <Private/Script/Internal/Utils.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scripting::CS
{
    Class::Class(
        MonoClass* Cls) :
        m_Class(Cls)
    {
        FetchMethods();
    }

    MonoClass* Class::GetClass() const
    {
        return m_Class;
    }

    Object Class::New(
        const char*  MethodName,
        const void** Parameters,
        size_t       ParameterCount) const
    {
        MonoObject* Obj = mono_object_new(ScriptContext::Get()->CurrentDomain, m_Class);

        MonoMethod* Ctor = GetMethod(MethodName);
        mono_runtime_invoke(Ctor, Obj, const_cast<void**>(Parameters), nullptr);

        return Obj;
    }

    Object Class::New() const
    {
        MonoObject* Obj = mono_object_new(ScriptContext::Get()->CurrentDomain, m_Class);
        mono_runtime_object_init(Obj);
        return Obj;
    }

    MonoMethod* Class::GetMethod(
        const StringU8& Name) const
    {
        return GetMethod(StringUtils::Hash(Name));
    }

    MonoMethod* Class::GetMethod(
        size_t NameHash) const
    {
        auto Iter = m_Methods.find(NameHash);
        return Iter != m_Methods.end() ? Iter->second : nullptr;
    }

    //

    void Class::FetchMethods()
    {
        void* Iter = nullptr;
        while (MonoMethod* Method = mono_class_get_methods(m_Class, &Iter))
        {
            // Get method name with arguments
            StringU8 MethodName = mono_method_get_name(Method);
            MethodName += "(";

            MonoMethodSignature* Signature = mono_method_signature(Method);
            void*                SigIter   = nullptr;
            uint32_t             i = 0, Count = mono_signature_get_param_count(Signature);
            while (MonoType* Type = mono_signature_get_params(Signature, &SigIter))
            {
                MethodName += mono_type_get_name(Type);
                if (i < Count - 1)
                {
                    MethodName += ", ";
                }
            }

            MethodName += ")";

            m_Methods.emplace(StringUtils::Hash(MethodName), Method);
        }
    }
} // namespace Neon::Scripting::CS