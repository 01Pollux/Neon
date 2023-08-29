#pragma once

#include <Core/String.hpp>

#include <Mono/jit/jit.h>
#include <Mono/metadata/appdomain.h>
#include <Mono/metadata/attrdefs.h>
#include <Mono/metadata/assembly.h>
#include <Mono/metadata/exception.h>
#include <Mono/metadata/mono-debug.h>
#include <Mono/metadata/mono-config.h>
#include <Mono/metadata/threads.h>
#include <Mono/metadata/tokentype.h>
#include <Mono/metadata/debug-helpers.h>
#include <Mono/metadata/class.h>
#include <Mono/metadata/object.h>

namespace Neon::Scripting::CS::Utils
{
    /// <summary>
    /// Get native object from mono object.
    /// </summary>
    template<typename _Ty>
    _Ty UnboxValue(
        MonoObject* Obj)
    {
        return *std::bit_cast<_Ty*>(mono_object_unbox(Obj));
    }

    /// <summary>
    /// Get mono object from native object.
    /// </summary>
    MonoObject* BoxValuePtr(
        MonoClass* Cls,
        void*      Ptr);

    /// <summary>
    /// Get mono object from native object.
    /// </summary>
    template<typename _Ty>
    MonoObject* BoxValue(
        MonoClass* Cls,
        _Ty        Obj)
    {
        return BoxValuePtr(Cls, std::bit_cast<void*>(std::addressof(Obj)));
    }

    //

    /// <summary>
    /// Handle mono exception.
    /// </summary>
    [[nodiscard]] void HandleException(
        MonoObject* Exception);

    /// <summary>
    /// Check mono error.
    /// Returns true if any error occured.
    /// </summary>
    [[nodiscard]] bool CheckMonoError(
        MonoError* Error);

    //

    /// <summary>
    /// Gets the name of the class.
    /// </summary>
    [[nodiscard]] StringU8 GetClassName(
        MonoClass* Class);

    /// <summary>
    /// Get empty mono string.
    /// </summary>
    [[nodiscard]] MonoString* GetEmptyMonoString();

    /// <summary>
    /// Get empty mono string.
    /// </summary>
    [[nodiscard]] MonoString* ToMonoString(
        const StringU8& Str);

    /// <summary>
    /// Get empty mono string.
    /// </summary>
    [[nodiscard]] StringU8 FromMonoString(
        MonoString* Str);
} // namespace Neon::Scripting::CS::Utils