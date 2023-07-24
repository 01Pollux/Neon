#pragma once

#include <Core/Neon.hpp>
#include <Crypto/SHA256.hpp>

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <map>

// clang-format off
#define NEON_MAPPED_BUFFER_ELEMENT_TYPES(Func)                          \
        Func(sizeof(bool), sizeof(long), Bool)                          \
        Func(sizeof(int16_t), sizeof(int16_t), Short)                   \
        Func(sizeof(uint16_t), sizeof(uint16_t), UShort)                \
        Func(sizeof(int32_t), sizeof(int32_t), Int)                     \
        Func(sizeof(int32_t[2]), sizeof(int32_t[2]), Int2)              \
        Func(sizeof(int32_t[3]), sizeof(int32_t[3]), Int3)              \
        Func(sizeof(int32_t[4]), sizeof(int32_t[4]), Int4)              \
        Func(sizeof(int32_t[3][3]), sizeof(int32_t[3][3]), Int3x3)      \
        Func(sizeof(int32_t[4][4]), sizeof(int32_t[4][4]), Int4x4)      \
        Func(sizeof(uint32_t), sizeof(uint32_t), UInt)                  \
        Func(sizeof(uint32_t[2]), sizeof(uint32_t[2]), UInt2)           \
        Func(sizeof(uint32_t[3]), sizeof(uint32_t[3]), UInt3)           \
        Func(sizeof(uint32_t[4]), sizeof(uint32_t[4]), UInt4)           \
        Func(sizeof(uint32_t[3][3]), sizeof(uint32_t[3][3]), UInt3x3)   \
        Func(sizeof(uint32_t[4][4]), sizeof(uint32_t[4][4]), UInt4x4)   \
        Func(sizeof(float), sizeof(float), Float)                       \
        Func(sizeof(double), sizeof(double), Double)                    \
        Func(sizeof(float[2]), sizeof(float[2]), Float2)                \
        Func(sizeof(float[3]), sizeof(float[3]), Float3)                \
        Func(sizeof(float[4]), sizeof(float[4]), Float4)                \
        Func(sizeof(float[3][3]), sizeof(float[3][3]), Float3x3)        \
        Func(sizeof(float[4][4]), sizeof(float[4][4]), Float4x4)
// clang-format on

namespace Neon::Structured
{
    enum class Type : uint8_t
    {
        None,
        Array,
        Struct,
#define NEON_MAPPED_BUFFER_FUNC(CPUSize, GPUSize, ElementType) ElementType,
        NEON_MAPPED_BUFFER_ELEMENT_TYPES(NEON_MAPPED_BUFFER_FUNC)
#undef NEON_MAPPED_BUFFER_FUNC

        Matrix = Float4x4,
    };

    namespace Impl
    {
        constexpr bool IsOutOfBounds(
            size_t StartOffset,
            size_t Size, size_t Alignement)
        {
            size_t EndOffset = StartOffset + Size;
            size_t PageStart = StartOffset / Alignement;
            size_t PageEnd   = EndOffset / Alignement;
            return (PageEnd != PageStart && (EndOffset % Alignement) != 0) || Size > Alignement;
        }

        constexpr size_t Align(
            size_t Size,
            size_t Alignement) noexcept
        {
            --Alignement;
            return (Size + Alignement) & ~Alignement;
        }

        constexpr size_t AlignIfOutOfBounds(
            size_t StartOffset,
            size_t Size,
            size_t Alignement) noexcept
        {
            return IsOutOfBounds(StartOffset, Size, Alignement) ? Align(Size, Alignement) : Size;
        }

        template<Type _Type>
        struct TypeMap
        {
            static_assert(sizeof(_Type) == -1, "Non-supported type is being used");
        };

#define NEON_MAPPED_BUFFER_FUNC(CPUSize, GPUSize, ElementType) \
    template<>                                                 \
    struct TypeMap<Type::ElementType>                          \
    {                                                          \
        static constexpr size_t SizeInGPU = GPUSize;           \
        static constexpr size_t SizeInCPU = CPUSize;           \
    };
        NEON_MAPPED_BUFFER_ELEMENT_TYPES(NEON_MAPPED_BUFFER_FUNC)
#undef NEON_MAPPED_BUFFER_FUNC
    } // namespace Impl

    //

    class CookedLayout;

    class RawLayout
    {
        friend class CookedLayout;

        class Element
        {
        public:
            struct StructData
            {
                std::vector<std::pair<std::string, Element>> NestedElements;
            };

            struct ArrayData
            {
                std::unique_ptr<Element> NestedElement;
                uint32_t                 ArrayCount;
            };

            struct RawData
            {
                Type NestedType;
            };

        public:
            Element(Type Type = Type::None);

            [[nodiscard]] StructData* AsStruct();

            [[nodiscard]] ArrayData* AsArray();

            [[nodiscard]] RawData* AsRaw();

            [[nodiscard]] const StructData* AsStruct() const;

            [[nodiscard]] const ArrayData* AsArray() const;

            [[nodiscard]] const RawData* AsRaw() const;

            [[nodiscard]] Type GetType() const noexcept;

        private:
            std::variant<StructData, ArrayData, RawData> m_Data;
        };

    public:
        class ElementView
        {
            friend class RawLayout;

        public:
            ElementView(
                Element* Element);

            /// <summary>
            /// Get element in struct layout
            /// </summary>
            [[nodiscard]] ElementView operator[](
                const std::string& Name);

            /// <summary>
            /// Append type to the layout
            /// </summary>
            ElementView Append(
                Type        Type,
                std::string Name);

            /// <summary>
            /// Append array to the layout
            /// </summary>
            ElementView AppendArray(
                Type        Type,
                std::string Name,
                uint32_t    ArrayCount);

            /// <summary>
            /// Get type of the array
            /// </summary>
            [[nodiscard]] ElementView GetArrayType() const;

            /// <summary>
            /// Append array to the layout
            /// </summary>
            void SetArrayType(
                Type     Type,
                uint32_t ArrayCount);

            /// <summary>
            /// Get type of the layout
            /// </summary>
            [[nodiscard]] Type GetType() const noexcept;

            /// <summary>
            /// Cast to struct
            /// </summary>
            [[nodiscard]] const Element::StructData* AsStruct() const noexcept;

            /// <summary>
            /// Cast to array
            /// </summary>
            [[nodiscard]] const Element::ArrayData* AsArray() const noexcept;

            /// <summary>
            /// Get hash code of the layout
            /// </summary>
            [[nodiscard]] void GetHashCode(
                Crypto::Sha256& Sha256) const;

        private:
            Element* m_Element;
        };

        static constexpr size_t ShaderAlignement = 16;

        RawLayout(
            size_t Alignement = 1u);

        NEON_CLASS_NO_COPY(RawLayout);
        NEON_CLASS_MOVE(RawLayout);
        ~RawLayout() = default;

        [[nodiscard]] ElementView GetView() noexcept
        {
            return ElementView(&m_Element);
        }

        [[nodiscard]] ElementView GetView() const noexcept
        {
            return ElementView(&m_Element);
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] ElementView operator[](
            const std::string& Name);

        /// <summary>
        /// Append type to the layout
        /// </summary>
        ElementView Append(
            Type        Type,
            std::string Name);

        /// <summary>
        /// Append array to the layout
        /// </summary>
        ElementView AppendArray(
            Type        Type,
            std::string Name,
            uint32_t    Size);

        /// <summary>
        /// Get alignement of the layout
        /// </summary>
        [[nodiscard]] uint16_t GetAlignement() const noexcept;

        /// <summary>
        /// Set alignement of the layout
        /// </summary>
        void SetAlignement(
            uint16_t Alignement) noexcept;

        /// <summary>
        /// Cook layout into compressed version of mapped offsets
        /// </summary>
        [[nodiscard]] CookedLayout Cook(
            bool GPULayout) const noexcept;

        /// <summary>
        /// Serialize layout
        /// </summary>
        [[nodiscard]] Crypto::Sha256::Bytes GetHashCode(
            bool GPULayout) const;

    private:
        mutable Element m_Element = Element(Type::Struct);
        uint16_t        m_Alignement;
    };

    class CookedLayout
    {
        class Element
        {
            friend class ElementView;

        public:
            struct StructData
            {
                std::map<std::string, Element> NestedElements;
                uint32_t                       StructSize;
            };

            struct ArrayData
            {
                std::unique_ptr<Element> NestedElement;
                uint32_t                 ArrayCount;
            };

            struct RawData
            {
                uint32_t TypeSize;
            };

            explicit Element(
                Type   Type,
                bool   IsGPULayout,
                size_t Offset);

            static void ProcessElement(
                bool                      GPULayout,
                CookedLayout::Element&    CookedElement,
                const RawLayout::Element& LayoutElement,
                size_t&                   AppendOffset,
                size_t                    Alignement);

            [[nodiscard]] StructData* AsStruct();

            [[nodiscard]] ArrayData* AsArray();

            [[nodiscard]] RawData* AsRaw();

            [[nodiscard]] const StructData* AsStruct() const;

            [[nodiscard]] const ArrayData* AsArray() const;

            [[nodiscard]] const RawData* AsRaw() const;

            [[nodiscard]] size_t GetOffset() const;

        private:
            std::variant<StructData, ArrayData, RawData>
                   m_Data;
            size_t m_Offset = 0;
        };

    public:
        class ElementView
        {
            friend class BufferView;
            friend class CBufferView;

        public:
            explicit ElementView(
                const Element* Element);

            /// <summary>
            /// Get element in struct layout
            /// </summary>
            [[nodiscard]] ElementView operator[](
                const std::string& Name) const;

            /// <summary>
            /// Get offset in main struct for layout
            /// </summary>
            [[nodiscard]] size_t GetOffset() const;

            /// <summary>
            /// Get size of layout
            /// </summary>
            [[nodiscard]] uint32_t GetSize() const noexcept;

            /// <summary>
            /// Get count of elements in the array, or 1 if the type is not array
            /// </summary>
            [[nodiscard]] uint32_t GetArrayCount() const noexcept;

        private:
            const Element* m_Element;
        };

        CookedLayout() = default;

        CookedLayout(
            bool                      GPULayout,
            size_t                    Alignement,
            const RawLayout::Element& LayoutElement);

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] ElementView operator[](
            const std::string& Name) const;

        /// <summary>
        /// Get size of layout
        /// </summary>
        [[nodiscard]] size_t GetSize() const noexcept;

        /// <summary>
        /// Access buffer view (non-const)
        /// </summary>
        [[nodiscard]] BufferView Access(
            void*  Buffer,
            size_t ArrayIndex = 0) const noexcept;

        /// <summary>
        /// Access buffer view (const)
        /// </summary>
        [[nodiscard]] CBufferView Access(
            const void* Buffer,
            size_t      ArrayIndex = 0) const noexcept;

    private:
        Element m_CookedLayout = Element(Type::Struct, 0, 0);
        size_t  m_Size         = 0;
    };

    //

    class BufferView
    {
    public:
        BufferView(
            uint8_t*                  Data,
            CookedLayout::ElementView View,
            size_t                    ArrayOffset) :
            m_Data(Data),
            m_View(View),
            m_ArrayOffset(ArrayOffset)
        {
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] BufferView operator[](
            const char* Name)
        {
            return BufferView(m_Data, m_View[Name], m_ArrayOffset);
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] BufferView operator[](
            const char* Name) const
        {
            return BufferView(m_Data, m_View[Name], m_ArrayOffset);
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] BufferView operator[](
            const std::string& Name)
        {
            return (*this)[Name.c_str()];
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] BufferView operator[](
            const std::string& Name) const
        {
            return (*this)[Name.c_str()];
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] BufferView operator[](
            size_t Offset)
        {
            auto Array = m_View.m_Element->AsArray();
            return BufferView(m_Data, CookedLayout::ElementView(Array->NestedElement.get()), m_ArrayOffset + Offset * m_View.GetSize());
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] BufferView operator[](
            size_t Offset) const
        {
            auto Array = m_View.m_Element->AsArray();
            return BufferView(m_Data, CookedLayout::ElementView(Array->NestedElement.get()), m_ArrayOffset + Offset * m_View.GetSize());
        }

        template<typename _Ty = uint8_t>
        const _Ty* As() const
        {
            return std::bit_cast<const _Ty*>(m_Data + m_View.GetOffset() + m_ArrayOffset);
        }

        template<typename _Ty = uint8_t>
        _Ty* As()
        {
            return std::bit_cast<_Ty*>(m_Data + m_View.GetOffset() + m_ArrayOffset);
        }

        template<typename _Ty>
        operator const _Ty&() const
        {
            return *Data<_Ty>();
        }

        template<typename _Ty>
        operator _Ty&()
        {
            return *As<_Ty>();
        }

        template<typename _Ty>
        const _Ty& operator=(const _Ty& Val) const
        {
            return *As<_Ty>() = Val;
        }

        template<typename _Ty>
        const _Ty& operator=(const _Ty& Val)
        {
            return *As<_Ty>() = Val;
        }

        /// <summary>
        /// Get offset of type in layout
        /// </summary>
        [[nodiscard]] size_t GetOffset() const
        {
            return m_View.GetOffset();
        }

        /// <summary>
        /// Get size of layout
        /// </summary>
        [[nodiscard]] uint32_t GetSize() const
        {
            return m_View.GetSize();
        }

        /// <summary>
        /// Get count of elements in the array, or 1 if the type is not array
        /// </summary>
        [[nodiscard]] uint32_t GetArrayCount() const
        {
            return m_View.GetArrayCount();
        }

    private:
        CookedLayout::ElementView m_View;
        size_t                    m_ArrayOffset;
        uint8_t*                  m_Data;
    };

    class CBufferView
    {
    public:
        CBufferView(
            const uint8_t*            Data,
            CookedLayout::ElementView View,
            size_t                    ArrayOffset) :
            m_Data(Data),
            m_View(View),
            m_ArrayOffset(ArrayOffset)
        {
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] CBufferView operator[](
            const char* Name) const
        {
            return CBufferView(m_Data, m_View[Name], m_ArrayOffset);
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] CBufferView operator[](
            const std::string& Name) const
        {
            return (*this)[Name.c_str()];
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] CBufferView operator[](
            size_t Offset) const
        {
            auto Array = m_View.m_Element->AsArray();
            return CBufferView(m_Data, CookedLayout::ElementView(Array->NestedElement.get()), m_ArrayOffset + Offset * m_View.GetSize());
        }

        template<typename _Ty = uint8_t>
        const _Ty* As() const
        {
            return std::bit_cast<const _Ty*>(m_Data + m_View.GetOffset() + m_ArrayOffset);
        }

        template<typename _Ty>
        operator const _Ty&() const
        {
            return *As<_Ty>();
        }

        template<typename _Ty>
        const _Ty& operator=(
            const _Ty& Val) const
        {
            return *As<_Ty>() = Val;
        }

        /// <summary>
        /// Get offset of type in layout
        /// </summary>
        [[nodiscard]] size_t GetOffset() const
        {
            return m_View.GetOffset();
        }

        /// <summary>
        /// Get size of layout
        /// </summary>
        [[nodiscard]] uint32_t GetSize() const
        {
            return m_View.GetSize();
        }

        /// <summary>
        /// Get count of elements in the array, or 1 if the type is not array
        /// </summary>
        [[nodiscard]] uint32_t GetArrayCount() const
        {
            return m_View.GetArrayCount();
        }

    private:
        CookedLayout::ElementView m_View;
        size_t                    m_ArrayOffset;
        const uint8_t*            m_Data;
    };
} // namespace Neon::Structured
