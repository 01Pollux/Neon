#pragma once

#include <memory>

#if NEON_DIST
#define NEON_NEW new
#else
#define NEON_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#define NEON_NOVTABLE __declspec(novtable)

// #define NEON_NO_LOG

#define NEON_CLASS_NO_COPY(Class)            \
    Class(const Class&)            = delete; \
    Class& operator=(const Class&) = delete

#define NEON_CLASS_COPY(Class)                \
    Class(const Class&)            = default; \
    Class& operator=(const Class&) = default

#define NEON_CLASS_NO_MOVE(Class)       \
    Class(Class&&)            = delete; \
    Class& operator=(Class&&) = delete

#define NEON_CLASS_MOVE(Class)           \
    Class(Class&&)            = default; \
    Class& operator=(Class&&) = default

#define NEON_CLASS_NO_COPYMOVE(Class) \
    NEON_CLASS_NO_COPY(Class);        \
    NEON_CLASS_NO_MOVE(Class)

#define NEON_CLASS_COPYMOVE(Class) \
    NEON_CLASS_COPY(Class);        \
    NEON_CLASS_MOVE(Class)

namespace Neon
{
    template<typename _Ty>
    using Ptr = std::shared_ptr<_Ty>;

    template<typename _Ty>
    using Ref = std::weak_ptr<_Ty>;

    template<typename _Ty, typename _Dx = std::default_delete<_Ty>>
    using UPtr = std::unique_ptr<_Ty, _Dx>;
} // namespace Neon