#pragma once

#include <type_traits>
#include <string>
#include "imgui/imgui.h"

namespace imcxx
{
    enum class scope_traits : char
    {
        no_dtor,
        has_dtor,
        always_dtor
    };

    template<typename _ParentTy, scope_traits _Traits = scope_traits::has_dtor, bool _HasReturn = true>
    class scope_wrap
    {
    public:
        using parent_type = _ParentTy;
        using this_type   = scope_wrap;

        static constexpr scope_traits traits_value = _Traits;
        static constexpr bool         has_dtor     = traits_value != scope_traits::no_dtor;
        static constexpr bool         has_return   = _HasReturn;

        explicit scope_wrap(bool res = true) noexcept :
            m_Result(res)
        {
        }

        ~scope_wrap()
        {
            try_destruct();
        }

        scope_wrap& operator=(const scope_wrap&) = delete;
        scope_wrap& operator=(scope_wrap&&)      = delete;
        scope_wrap(const scope_wrap&)            = delete;
        scope_wrap(scope_wrap&& o) noexcept :
            m_Result(o.m_Result)
        {
            if (!o.m_Result._HasMoved)
                o.m_Result._HasMoved = true;
        }

    public:
        template<typename... _Args>
        static parent_type call(_Args&&... args)
        {
            return parent_type{ std::forward<_Args>(args)... };
        }

        [[nodiscard]] operator bool() const noexcept
        {
            if constexpr (has_return)
                return m_Result._Value;
            else
                return true;
        }

        template<typename _InvokeTy, typename... _Args>
        const parent_type& active_invoke(_InvokeTy&& fn, _Args&&... args) const noexcept(std::is_nothrow_invocable_v<_InvokeTy, _Args...>)
        {
            if (*this)
            {
                std::invoke(std::forward<_InvokeTy>(fn), std::forward<_Args>(args)...);
            }
            return *static_cast<const parent_type*>(this);
        }

        template<typename _InvokeTy, typename... _Args>
        const parent_type& inactive_invoke(_InvokeTy&& fn, _Args&&... args) const noexcept(std::is_nothrow_invocable_v<_InvokeTy, _Args...>)
        {
            if (*this)
            {
                std::invoke(std::forward<_InvokeTy>(fn), std::forward<_Args>(args)...);
            }
            return *static_cast<const parent_type*>(this);
        }

        template<typename _InvokeTy, typename... _Args>
        const parent_type& passive_invoke(_InvokeTy&& fn, _Args&&... args) const noexcept(std::is_nothrow_invocable_v<_InvokeTy, _Args...>)
        {
            std::invoke(std::forward<_InvokeTy>(fn), std::forward<_Args>(args)...);
            return *static_cast<const parent_type*>(this);
        }

        template<typename _InvokeTy>
        const parent_type& operator<<(_InvokeTy&& fn) const noexcept(std::is_nothrow_invocable_v<_InvokeTy>)
        {
            return active_invoke(std::forward<_InvokeTy>(fn));
        }

        template<typename _InvokeTy, typename... _Args>
        parent_type& active_invoke(_InvokeTy&& fn, _Args&&... args)
        {
            if (*this)
            {
                std::invoke(std::forward<_InvokeTy>(fn), std::forward<_Args>(args)...);
            }
            return *static_cast<parent_type*>(this);
        }

        template<typename _InvokeTy, typename... _Args>
        parent_type& inactive_invoke(_InvokeTy&& fn, _Args&&... args) noexcept(std::is_nothrow_invocable_v<_InvokeTy, _Args...>)
        {
            if (!*this)
            {
                std::invoke(std::forward<_InvokeTy>(fn), std::forward<_Args>(args)...);
            }
            return *static_cast<parent_type*>(this);
        }

        template<typename _InvokeTy, typename... _Args>
        parent_type& passive_invoke(_InvokeTy&& fn, _Args&&... args) noexcept(std::is_nothrow_invocable_v<_InvokeTy, _Args...>)
        {
            std::invoke(std::forward<_InvokeTy>(fn), std::forward<_Args>(args)...);
            return *static_cast<parent_type*>(this);
        }

        template<typename _InvokeTy>
        parent_type& operator<<(_InvokeTy&& fn) noexcept(std::is_nothrow_invocable_v<_InvokeTy>)
        {
            return active_invoke(std::forward<_InvokeTy>(fn));
        }

    protected:
        template<bool _HasBool = true>
        struct _compressed_type_t
        {
            bool _HasMoved : 1;
            bool _Value    : 1;
            constexpr _compressed_type_t(bool v) noexcept :
                _Value(v), _HasMoved(false)
            {
            }
        };

        template<>
        struct _compressed_type_t<false>
        {
            bool _HasMoved : 1;
            constexpr _compressed_type_t(bool) noexcept :
                _HasMoved(false)
            {
            }
        };

        _compressed_type_t<has_return> m_Result;

    private:
        void try_destruct()
        {
            if (m_Result._HasMoved)
                return;

            if constexpr (has_dtor)
            {
                if constexpr (traits_value != scope_traits::always_dtor)
                {
                    if constexpr (has_return)
                    {
                        if (!m_Result._Value)
                            return;
                    }
                }
                static_cast<parent_type*>(this)->destruct();
            }
        }
    };

    namespace impl
    {
        template<typename T>
        static constexpr const char* default_c_format = "";

        template<typename _Ty>
        static constexpr bool is_imvec4_v = std::is_same_v<_Ty, ImVec4>;

        template<typename _Ty>
        static constexpr bool is_imvec2_v = std::is_same_v<_Ty, ImVec2>;

        template<typename _ArrTy>
        struct array_value
        {
            using type                   = std::remove_pointer_t<std::decay_t<_ArrTy>>;
            static constexpr size_t size = std::extent_v<_ArrTy>;
        };

        template<typename _Ty, size_t _Size>
        struct array_value<std::array<_Ty, _Size>>
        {
            using type                   = _Ty;
            static constexpr size_t size = _Size;
        };

        template<typename _Ty>
        using array_value_t = typename array_value<_Ty>::type;

        template<typename _Ty>
        static constexpr size_t array_value_size = array_value<_Ty>::size;

        template<>
        struct array_value<ImVec4>
        {
            using type                   = float;
            static constexpr size_t size = 4;
        };

        template<>
        struct array_value<ImVec2>
        {
            using type                   = float;
            static constexpr size_t size = 2;
        };

        template<typename _Ty>
        [[nodiscard]] constexpr ImVec2 to_imvec2(const _Ty& vec) noexcept
        {
            if constexpr (is_imvec2_v<_Ty>)
                return vec;
            else if constexpr (is_imvec4_v<_Ty>)
                return { vec.x, vec.y };
            else if constexpr (
                std::is_same_v<std::array<float, 2>, _Ty> ||
                std::is_same_v<std::array<float, 4>, _Ty> ||
                std::is_same_v<std::initializer_list<float>, _Ty> && vec.size() >= 2 ||
                std::is_same_v<float[2], _Ty>)
                return { vec[0], vec[1] };
            else
                return static_cast<ImVec2>(vec);
        }

        template<typename _Ty>
        [[nodiscard]] constexpr ImVec4 to_imvec4(const _Ty& vec) noexcept
        {
            if constexpr (is_imvec4_v<_Ty>)
                return vec;
            else if constexpr (
                std::is_same_v<std::array<float, 4>, _Ty> ||
                std::is_same_v<std::initializer_list<float>, _Ty> && vec.size() >= 4 ||
                std::is_same_v<float[4], _Ty>)
                return { vec[0], vec[1], vec[2], vec[3] };
            else
                return static_cast<ImVec4>(vec);
        }

        template<typename _Ty>
        [[nodiscard]] static constexpr const char* get_string(const _Ty& str) noexcept
        {
            if constexpr (std::is_same_v<char*, std::decay_t<_Ty>> || std::is_same_v<const char*, std::decay_t<_Ty>>)
                return str;
            else if constexpr (std::is_null_pointer_v<_Ty>)
                return "";
            else
            {
                if constexpr (std::is_pointer_v<_Ty>)
                    return str->c_str();
                else
                    return str.c_str();
            }
        }

        template<typename _Ty>
        [[nodiscard]] static constexpr ImGuiDataType to_imdatatype() noexcept
        {
            if constexpr (std::is_same_v<_Ty, char>)
                return ImGuiDataType_S8;
            else if constexpr (std::is_same_v<_Ty, uint8_t>)
                return ImGuiDataType_U8;
            else if constexpr (std::is_same_v<_Ty, int16_t>)
                return ImGuiDataType_S16;
            else if constexpr (std::is_same_v<_Ty, uint16_t>)
                return ImGuiDataType_U16;
            else if constexpr (std::is_same_v<_Ty, int32_t>)
                return ImGuiDataType_S32;
            else if constexpr (std::is_same_v<_Ty, uint32_t>)
                return ImGuiDataType_U32;
            else if constexpr (std::is_same_v<_Ty, int64_t>)
                return ImGuiDataType_S64;
            else if constexpr (std::is_same_v<_Ty, uint64_t>)
                return ImGuiDataType_U64;
            else if constexpr (std::is_same_v<_Ty, float>)
                return ImGuiDataType_Float;
            else if constexpr (std::is_same_v<_Ty, double>)
                return ImGuiDataType_Double;
            else
                return ImGuiDataType_COUNT;
        }

        template<>
        constexpr const char* default_c_format<char> = "%d";
        template<>
        constexpr const char* default_c_format<uint8_t> = "%u";
        template<>
        constexpr const char* default_c_format<int16_t> = "%d";
        template<>
        constexpr const char* default_c_format<uint16_t> = "%u";
        template<>
        constexpr const char* default_c_format<int32_t> = "%d";
        template<>
        constexpr const char* default_c_format<uint32_t> = "%u";
#ifdef _MSC_VER
        template<>
        constexpr const char* default_c_format<int64_t> = "%I64d";
        template<>
        constexpr const char* default_c_format<uint64_t> = "%I64u";
#else
        template<>
        constexpr const char* default_c_format<int64_t> = "%lld";
        template<>
        constexpr const char* default_c_format<uint64_t> = "%llu";
#endif
        template<>
        constexpr const char* default_c_format<float> = "%.3f";
        template<>
        constexpr const char* default_c_format<double> = "%.6lf";
    } // namespace impl
} // namespace imcxx
