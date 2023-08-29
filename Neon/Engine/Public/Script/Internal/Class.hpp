#pragma once

#include <Script/Internal/Object.hpp>
#include <Mono/metadata/object.h>
#include <Asio/Coroutines.hpp>

typedef struct _MonoClass  MonoClass;
typedef struct _MonoMethod MonoMethod;

namespace Neon::Scripting::CS
{
    template<typename _Ty>
    struct ObjectInvokerArg
    {
        static void* ToPtr(
            _Ty& Obj)
        {
            return std::addressof(Obj);
        }

        static _Ty FromPtr(
            MonoObject* Obj)
        {
            return *std::bit_cast<_Ty*>(mono_object_unbox(Obj));
        }
    };

    //

    template<>
    struct ObjectInvokerArg<StringU8>
    {
        static void* ToPtr(
            StringU8& Obj);

        static StringU8 FromPtr(
            MonoObject* Obj);
    };

    //

    template<>
    struct ObjectInvokerArg<MonoObject*>
    {
        static void* ToPtr(
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

    struct MethodMetadata
    {
        MonoMethod*           Method;
        std::vector<StringU8> Parameters;
    };

    class Class
    {
        friend class Assembly;

    public:
        Class(
            MonoClass* Cls);

        /// <summary>
        /// Gets the assembly.
        /// </summary>
        [[nodiscard]] const Class* GetParent() const;

        /// <summary>
        /// Gets the assembly.
        /// </summary>
        [[nodiscard]] MonoClass* GetClass() const;

        /// <summary>
        /// Construct a new object with the specified constructor.
        /// </summary>
        MonoObject* Invoke(
            MonoMethod*  Method,
            MonoObject*  Obj,
            const void** Parameters,
            size_t       ParameterCount) const;

        /// <summary>
        /// Construct a new object with the specified constructor.
        /// </summary>
        [[nodiscard]] Object New(
            std::span<const char*> ParameterTypes,
            const void**           Parameters,
            size_t                 ParameterCount) const;

        /// <summary>
        /// Construct a new object with default constructor.
        /// </summary>
        [[nodiscard]] Object New() const;

        /// <summary>
        /// Gets the method with the specified name.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<const MethodMetadata&> GetMethods(
            const char* Name) const;

        /// <summary>
        /// Gets the method with the specified name.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<const MethodMetadata&> GetMethods(
            size_t NameHash) const;

    public:
        /// <summary>
        /// Gets the method with the specified name and parameter types.
        /// </summary>
        [[nodiscard]] MonoMethod* FindMethod(
            const char*            Name,
            std::span<const char*> ParameterTypes) const;

        /// <summary>
        /// Gets the method with the specified name and parameter types.
        /// </summary>
        [[nodiscard]] MonoMethod* FindMethod(
            size_t                 NameHash,
            std::span<const char*> ParameterTypes) const;

    private:
        /// <summary>
        /// Fetches all the methods in the class.
        /// </summary>
        void FetchMethods();

    private:
        MonoClass*   m_Class;
        const Class* m_BaseClass = nullptr;

        std::unordered_multimap<size_t, MethodMetadata> m_Methods;
    };

    template<typename _RetTy, typename... _Args>
    struct MethodInvoker
    {
    public:
        MethodInvoker(
            const Class* Cls    = nullptr,
            MonoMethod*  Method = nullptr) :
            m_Class(Cls),
            m_Method(Method)
        {
        }

        [[nodiscard]] _RetTy operator()(
            _Args&&... Args)
        {
            return Invoke(nullptr, std::forward<_Args>(Args)...);
        }

        [[nodiscard]] _RetTy operator()(
            MonoObject* Obj,
            _Args&&... Args)
        {
            return Invoke(Obj, std::forward<_Args>(Args)...);
        }

    private:
        [[nodiscard]] _RetTy Invoke(
            MonoObject* Obj,
            _Args&&... Args)
        {
            const void* Parameters[] = { ObjectInvokerArg<_Args>::ToPtr(Args)... };

            if constexpr (std::is_same_v<_RetTy, void>)
            {
                m_Class->Invoke(m_Method, Obj, Parameters, sizeof...(Args));
            }
            else
            {
                return ObjectInvokerArg<_RetTy>::FromPtr(
                    m_Class->Invoke(m_Method, Obj, Parameters, sizeof...(Args)));
            }
        }

    private:
        const Class* m_Class;
        MonoMethod*  m_Method;
    };
} // namespace Neon::Scripting::CS