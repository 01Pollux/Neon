#include <CorePCH.hpp>
#include <Utils/Struct.hpp>

#include <Log/Logger.hpp>

namespace Neon::Structured
{
    RawLayout::Element::Element(Type Type)
    {
        switch (Type)
        {
        case Type::None:
            break;
        case Type::Array:
            m_Data = ArrayData{};
            break;
        case Type::Struct:
            m_Data = StructData{};
            break;
        default:
            m_Data = RawData{ Type };
        }
    }

    auto RawLayout::Element::AsStruct() -> StructData*
    {
        return std::holds_alternative<StructData>(m_Data) ? &std::get<StructData>(m_Data) : nullptr;
    }

    auto RawLayout::Element::AsArray() -> ArrayData*
    {
        return std::holds_alternative<ArrayData>(m_Data) ? &std::get<ArrayData>(m_Data) : nullptr;
    }

    auto RawLayout::Element::AsRaw() -> RawData*
    {
        return std::holds_alternative<RawData>(m_Data) ? &std::get<RawData>(m_Data) : nullptr;
    }

    auto RawLayout::Element::AsStruct() const -> const StructData*
    {
        return std::holds_alternative<StructData>(m_Data) ? &std::get<StructData>(m_Data) : nullptr;
    }

    auto RawLayout::Element::AsArray() const -> const ArrayData*
    {
        return std::holds_alternative<ArrayData>(m_Data) ? &std::get<ArrayData>(m_Data) : nullptr;
    }

    auto RawLayout::Element::AsRaw() const -> const RawData*
    {
        return std::holds_alternative<RawData>(m_Data) ? &std::get<RawData>(m_Data) : nullptr;
    }

    Type RawLayout::Element::GetType() const noexcept
    {
        // clang-format off
        return
            std::holds_alternative<StructData>(m_Data) ? Type::Struct : 
            std::holds_alternative<ArrayData>(m_Data) ? Type::Array :
            std::get<RawData>(m_Data).NestedType;
        // clang-format on
    }

    RawLayout::ElementView::ElementView(
        Element* Element) :
        m_Element(Element)
    {
        NEON_ASSERT(Element, "Tried to access a null element view");
    }

    auto RawLayout::ElementView::operator[](
        const std::string& Name) -> ElementView
    {
        Element::StructData* Struct = m_Element->AsStruct();
        NEON_ASSERT(Struct, "Tried to access view of non-struct element");

        for (auto& NestedElement : Struct->NestedElements)
        {
            if (NestedElement.first == Name)
            {
                return ElementView(&NestedElement.second);
            }
        }
        return nullptr;
    }

    auto RawLayout::ElementView::Append(
        Type        Type,
        std::string Name) -> ElementView
    {
        Element::StructData* Struct = m_Element->AsStruct();
        NEON_ASSERT(Struct, "Tried to access view of non-struct element");

        auto& NewElement = Struct->NestedElements.emplace_back(std::move(Name), Element(Type));
        return ElementView(&NewElement.second);
    }

    auto RawLayout::ElementView::AppendArray(
        Type        Type,
        std::string Name,
        uint32_t    ArrayCount) -> ElementView
    {
        Element::StructData* Struct = m_Element->AsStruct();
        NEON_ASSERT(Struct, "Tried to access view of non-struct element");

        auto Array = Struct->NestedElements.emplace_back(std::move(Name), Element(Type::Array)).second.AsArray();

        Array->ArrayCount    = ArrayCount;
        Array->NestedElement = std::make_unique<Element>(Type);

        return ElementView(Array->NestedElement.get());
    }

    auto RawLayout::ElementView::GetArrayType() const -> ElementView
    {
        Element::ArrayData* Array = m_Element->AsArray();
        NEON_ASSERT(Array, "Tried to access view of non-array element");

        return ElementView(Array->NestedElement.get());
    }

    void RawLayout::ElementView::SetArrayType(
        Type     Type,
        uint32_t ArrayCount)
    {
        Element::ArrayData* Array = m_Element->AsArray();
        NEON_ASSERT(Array, "Tried to access view of non-array element");

        Array->ArrayCount    = ArrayCount;
        Array->NestedElement = std::make_unique<Element>(Type);
    }

    Type RawLayout::ElementView::GetType() const noexcept
    {
        return m_Element->GetType();
    }

    auto RawLayout::ElementView::AsStruct() const noexcept -> const Element::StructData*
    {
        return m_Element->AsStruct();
    }

    auto RawLayout::ElementView::AsArray() const noexcept -> const Element::ArrayData*
    {
        return m_Element->AsArray();
    }

    void RawLayout::ElementView::GetHashCode(
        SHA256& Sha256) const
    {
        auto NestedType = m_Element->GetType();
        Sha256.Append(std::bit_cast<uint8_t*>(&NestedType), sizeof(NestedType));

        if (auto Struct = m_Element->AsStruct())
        {
            for (auto& NestedElement : Struct->NestedElements)
            {
                ElementView(&NestedElement.second).GetHashCode(Sha256);
            }
        }
        else if (auto Array = m_Element->AsArray())
        {
            Sha256.Append(Array->ArrayCount);
            ElementView(Array->NestedElement.get()).GetHashCode(Sha256);
        }
    }

    RawLayout::RawLayout(size_t Alignement) :
        m_Alignement(uint16_t(Alignement))
    {
    }

    auto RawLayout::operator[](
        const std::string& Name) -> ElementView
    {
        return GetView()[Name];
    }

    auto RawLayout::Append(
        Type        Type,
        std::string Name) -> ElementView
    {
        return GetView().Append(Type, std::move(Name));
    }

    auto RawLayout::AppendArray(
        Type        Type,
        std::string Name,
        uint32_t    Size) -> ElementView
    {
        return GetView().AppendArray(Type, std::move(Name), Size);
    }

    uint16_t RawLayout::GetAlignement() const noexcept
    {
        return m_Alignement;
    }

    void RawLayout::SetAlignement(
        uint16_t Alignement) noexcept
    {
        m_Alignement = Alignement;
    }

    CookedLayout RawLayout::Cook(
        bool GPULayout) const noexcept
    {
        return CookedLayout(GPULayout, GetAlignement(), m_Element);
    }

    SHA256::Bytes RawLayout::GetHashCode(
        bool GPULayout) const
    {
        SHA256 Sha256;

        Sha256.Append(GPULayout);
        Sha256.Append(m_Alignement);

        ElementView(const_cast<Element*>(&m_Element)).GetHashCode(Sha256);
        return Sha256.Digest();
    }

    //

    CookedLayout::Element::Element(
        Type   Type,
        bool   IsGPULayout,
        size_t Offset) :
        m_Offset(Offset)
    {
        switch (Type)
        {
        case Type::None:
            break;
        case Type::Array:
            m_Data = ArrayData{};
            break;
        case Type::Struct:
            m_Data = StructData{};
            break;
        default:
            switch (Type)
            {
#define NEON_MAPPED_BUFFER_FUNC(CPUSize, GPUSize, ElementType) \
    case Type::ElementType:                                    \
    {                                                          \
        if (IsGPULayout)                                       \
            m_Data = RawData{ GPUSize };                       \
        else                                                   \
            m_Data = RawData{ CPUSize };                       \
        break;                                                 \
    }

                NEON_MAPPED_BUFFER_ELEMENT_TYPES(NEON_MAPPED_BUFFER_FUNC)
#undef NEON_MAPPED_BUFFER_FUNC
            }
        }
    }

    auto CookedLayout::Element::AsStruct() -> StructData*
    {
        return std::holds_alternative<StructData>(m_Data) ? &std::get<StructData>(m_Data) : nullptr;
    }

    auto CookedLayout::Element::AsArray() -> ArrayData*
    {
        return std::holds_alternative<ArrayData>(m_Data) ? &std::get<ArrayData>(m_Data) : nullptr;
    }

    auto CookedLayout::Element::AsRaw() -> RawData*
    {
        return std::holds_alternative<RawData>(m_Data) ? &std::get<RawData>(m_Data) : nullptr;
    }

    auto CookedLayout::Element::AsStruct() const -> const StructData*
    {
        return std::holds_alternative<StructData>(m_Data) ? &std::get<StructData>(m_Data) : nullptr;
    }

    auto CookedLayout::Element::AsArray() const -> const ArrayData*
    {
        return std::holds_alternative<ArrayData>(m_Data) ? &std::get<ArrayData>(m_Data) : nullptr;
    }

    auto CookedLayout::Element::AsRaw() const -> const RawData*
    {
        return std::holds_alternative<RawData>(m_Data) ? &std::get<RawData>(m_Data) : nullptr;
    }

    size_t CookedLayout::Element::GetOffset() const
    {
        return m_Offset;
    }

    void CookedLayout::Element::ProcessElement(
        bool                      IsGPULayout,
        CookedLayout::Element&    CookedElement,
        const RawLayout::Element& LayoutElement,
        size_t&                   AppendOffset,
        size_t                    Alignement)
    {
        switch (LayoutElement.GetType())
        {
        case Type::Array:
        {
            auto Array       = LayoutElement.AsArray();
            auto CookedArray = CookedElement.AsArray();
            NEON_ASSERT(Array->NestedElement, "Invalid nested element in CookedLayout");

            CookedArray->ArrayCount    = Array->ArrayCount;
            CookedArray->NestedElement = std::make_unique<CookedLayout::Element>(
                Array->NestedElement->GetType(), 0, AppendOffset);

            size_t CurOffset = AppendOffset;
            ProcessElement(
                IsGPULayout,
                *CookedArray->NestedElement,
                *Array->NestedElement,
                AppendOffset,
                Alignement);

            size_t Size  = (AppendOffset - CurOffset) * Array->ArrayCount;
            AppendOffset = CurOffset + Size;

            break;
        }
        case Type::Struct:
        {
            auto Struct       = LayoutElement.AsStruct();
            auto CookedStruct = CookedElement.AsStruct();

            for (auto& NestedElement : Struct->NestedElements)
            {
                // clang-format off
                auto& CookedNestedElement = 
                    *CookedStruct->NestedElements.emplace(
                        NestedElement.first,
                        CookedLayout::Element(NestedElement.second.GetType(), IsGPULayout, AppendOffset))
                    .first;
                // clang-format on

                size_t CurOffset = AppendOffset;
                ProcessElement(
                    IsGPULayout,
                    CookedNestedElement.second,
                    NestedElement.second,
                    AppendOffset,
                    Alignement);

                size_t Size = AppendOffset - CurOffset;
                CookedStruct->StructSize += uint32_t(Size);
            }

            break;
        }

#define NEON_MAPPED_BUFFER_FUNC(CPUSize, GPUSize, ElementType)                                     \
    case Type::ElementType:                                                                        \
    {                                                                                              \
        CookedElement = CookedLayout::Element(LayoutElement.GetType(), IsGPULayout, AppendOffset); \
        if (IsGPULayout)                                                                           \
            AppendOffset += Impl::AlignIfOutOfBounds(AppendOffset, GPUSize, Alignement);           \
        else                                                                                       \
            AppendOffset += Impl::AlignIfOutOfBounds(AppendOffset, CPUSize, Alignement);           \
        break;                                                                                     \
    }

            NEON_MAPPED_BUFFER_ELEMENT_TYPES(NEON_MAPPED_BUFFER_FUNC)
#undef NEON_MAPPED_BUFFER_FUNC
        default:
            NEON_ASSERT(false, "Invalid type in CookedLayout");
            break;
        }
    }

    //

    CookedLayout::ElementView::ElementView(
        const Element* Element) :
        m_Element(Element)
    {
    }

    auto CookedLayout::ElementView::operator[](
        const std::string& Name) const -> ElementView
    {
        const Element::StructData* Struct = m_Element->AsStruct();
        NEON_ASSERT(Struct, "Tried to access view of non-struct element");
        return ElementView(&Struct->NestedElements.find(Name)->second);
    }

    size_t CookedLayout::ElementView::GetOffset() const
    {
        return m_Element->GetOffset();
    }

    uint32_t CookedLayout::ElementView::GetSize() const noexcept
    {
        if (auto Raw = m_Element->AsRaw())
        {
            return Raw->TypeSize;
        }
        else if (auto Struct = m_Element->AsStruct())
        {
            return Struct->StructSize;
        }
        else
        {
            auto Array = m_Element->AsArray();
            return ElementView(Array->NestedElement.get()).GetSize();
        }
    }

    uint32_t CookedLayout::ElementView::GetArrayCount() const noexcept
    {
        auto Array = m_Element->AsArray();
        return Array ? Array->ArrayCount : 1;
    }

    //

    CookedLayout::CookedLayout(
        bool                      GPULayout,
        size_t                    Alignement,
        const RawLayout::Element& LayoutElement)
    {
        Element::ProcessElement(GPULayout, m_CookedLayout, LayoutElement, m_Size, Alignement);
    }

    auto CookedLayout::operator[](
        const std::string& Name) const -> ElementView
    {
        return ElementView(&m_CookedLayout)[Name];
    }

    size_t CookedLayout::GetSize() const noexcept
    {
        return m_Size;
    }

    BufferView CookedLayout::Access(
        void*  Buffer,
        size_t ArrayIndex) const noexcept
    {
        return BufferView(static_cast<uint8_t*>(Buffer), ElementView(&m_CookedLayout), ArrayIndex * GetSize());
    }

    CBufferView CookedLayout::Access(
        const void* Buffer,
        size_t      ArrayIndex) const noexcept
    {
        return CBufferView(static_cast<const uint8_t*>(Buffer), ElementView(&m_CookedLayout), ArrayIndex * GetSize());
    }
} // namespace Neon::Structured
