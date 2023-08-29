#include <EnginePCH.hpp>
#include <Private/Script/Internal/Object.hpp>

namespace Neon::Scripting::CS
{
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
} // namespace Neon::Scripting::CS