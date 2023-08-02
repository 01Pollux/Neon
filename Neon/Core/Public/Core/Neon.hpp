#pragma once

#include <memory>

namespace Neon
{
    template<class... _Ty>
    struct VariantVisitor : _Ty...
    {
        using _Ty::operator()...;
    };
} // namespace Neon

#ifdef NEON_DIST
#define NEON_NEW new
#else
#define NEON_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

template<typename _To, typename _From>
[[nodiscard]] _To validate_cast(_From* From)
{
#ifdef NEON_DEBUG
    return dynamic_cast<_To>(From);
#else
    return static_cast<_To>(From);
#endif
}

// #define NEON_NO_LOG

//

#define NEON_CLASS_COPY_DECL(Class) \
    Class(const Class&);            \
    Class& operator=(const Class&)

//

#define NEON_CLASS_NO_COPY(Class)            \
    Class(const Class&)            = delete; \
    Class& operator=(const Class&) = delete

#define NEON_CLASS_NO_COPY_IMPL(Class)              \
    Class::Class(const Class&)            = delete; \
    Class& Class::operator=(const Class&) = delete

//

#define NEON_CLASS_COPY(Class)                \
    Class(const Class&)            = default; \
    Class& operator=(const Class&) = default

#define NEON_CLASS_COPY_IMPL(Class)                  \
    Class::Class(const Class&)            = default; \
    Class& Class::operator=(const Class&) = default

//
//

#define NEON_CLASS_MOVE_DECL(Class) \
    Class(Class&&);                 \
    Class& operator=(Class&&)

//

#define NEON_CLASS_NO_MOVE(Class)       \
    Class(Class&&)            = delete; \
    Class& operator=(Class&&) = delete

#define NEON_CLASS_NO_MOVE_IMPL(Class)         \
    Class::Class(Class&&)            = delete; \
    Class& Class::operator=(Class&&) = delete

//

#define NEON_CLASS_MOVE(Class)           \
    Class(Class&&)            = default; \
    Class& operator=(Class&&) = default

#define NEON_CLASS_MOVE_IMPL(Class)             \
    Class::Class(Class&&)            = default; \
    Class& Class::operator=(Class&&) = default

//
//

#define NEON_CLASS_NO_COPYMOVE(Class) \
    NEON_CLASS_NO_COPY(Class);        \
    NEON_CLASS_NO_MOVE(Class)

#define NEON_CLASS_NO_COPYMOVE_IMPL(Class) \
    NEON_CLASS_NO_COPY_IMPL(Class);        \
    NEON_CLASS_NO_MOVE_IMPL(Class)

#define NEON_CLASS_COPYMOVE(Class) \
    NEON_CLASS_COPY(Class);        \
    NEON_CLASS_MOVE(Class)

#define NEON_CLASS_COPYMOVE_IMPL(Class) \
    NEON_CLASS_COPY_IMPL(Class);        \
    NEON_CLASS_MOVE_IMPL(Class)

namespace Neon
{
    template<typename _Ty>
    using Ptr = std::shared_ptr<_Ty>;

    template<typename _Ty>
    using Ref = std::weak_ptr<_Ty>;

    template<typename _Ty, typename _Dx = std::default_delete<_Ty>>
    using UPtr = std::unique_ptr<_Ty, _Dx>;
} // namespace Neon

//

// Pure virtual on debug, disabled on release+dist
#if NEON_DEBUG
#define NEON_M_DEBUG_ONLY_PV(...) virtual __VA_ARGS__ = 0
#else
#define NEON_M_DEBUG_ONLY_PV(...) \
    __VA_ARGS__                   \
    {                             \
    }
#endif

// Pure virtual on debug+dist, disabled on release
#ifndef NEON_DIST
#define NEON_M_NODIST_PV(...) virtual __VA_ARGS__ = 0
#else
#define NEON_M_NODIST_PV(...) \
    __VA_ARGS__               \
    {                         \
    }
#endif

//

// Virtual on debug, disabled on release+dist
#if NEON_DEBUG
#define NEON_M_DEBUG_ONLY_V(...) virtual __VA_ARGS__
#else
#define NEON_M_DEBUG_ONLY_V(...) \
    __VA_ARGS__                  \
    {                            \
    }
#endif

// Virtual on debug+dist, disabled on release
#ifndef NEON_DIST
#define NEON_M_NODIST_V(...) virtual __VA_ARGS__
#else
#define NEON_M_NODIST_V(...) \
    __VA_ARGS__              \
    {                        \
    }
#endif

//

// Enabled on debug, disabled on release+dist
#if NEON_DEBUG
#define NEON_M_DEBUG_ONLY(...) __VA_ARGS__
#else
#define NEON_M_DEBUG_ONLY(...) \
    __VA_ARGS__                \
    {                          \
    }
#endif

// Enabled on debug+dist, disabled on release
#ifndef NEON_DIST
#define NEON_M_NODIST(...) __VA_ARGS__
#else
#define NEON_M_NODIST(...) \
    __VA_ARGS__            \
    {                      \
    }
#endif
