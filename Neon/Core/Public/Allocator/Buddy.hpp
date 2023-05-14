#pragma once

#include <Math/Common.hpp>
#include <map>

namespace Neon::Allocator
{
    class BuddyAllocator
    {
    public:
        struct Handle
        {
            size_t Offset = std::numeric_limits<size_t>::max();
            size_t Size   = 0;

            operator bool() const noexcept
            {
                return Size != 0;
            }
        };

        explicit BuddyAllocator(
            size_t Size)
        {
            m_FreeOffsets.emplace(0, Size);
            m_FreeSizes.emplace(Size, m_FreeOffsets.begin());
        }

        [[nodiscard]] Handle Allocate(
            size_t Size,
            size_t Alignement = 1)
        {
            Size = Math::AlignUp(Size, Alignement);

            Handle Hndl;
            auto   It = m_FreeSizes.lower_bound(Size);
            while (It != m_FreeSizes.end())
            {
                size_t OldOffset   = It->second->first;
                size_t Offset      = Math::AlignUp(OldOffset, Alignement);
                size_t DeltaOffset = Offset - OldOffset;
                size_t AlignedSize = It->first - DeltaOffset;

                if (AlignedSize < Size)
                {
                    ++It;
                    continue;
                }

                Hndl.Size   = Size;
                Hndl.Offset = Offset;

                m_FreeOffsets.erase(It->second);
                m_FreeSizes.erase(It);

                if (AlignedSize != Size)
                {
                    size_t NewSize = AlignedSize - Size;
                    auto   Iter    = m_FreeOffsets.emplace(Hndl.Offset + Size, NewSize);
                    m_FreeSizes.emplace(NewSize, Iter.first);
                }

                if (DeltaOffset)
                {
                    auto Iter = m_FreeOffsets.emplace(OldOffset, DeltaOffset);
                    m_FreeSizes.emplace(DeltaOffset, Iter.first);
                }
                break;
            }
            return Hndl;
        }

        void Free(
            Handle Hndl)
        {
            auto It = m_FreeOffsets.lower_bound(Hndl.Offset);
            if (It != m_FreeOffsets.end())
            {
                if (It->first == Hndl.Offset + Hndl.Size)
                {
                    Hndl.Size += It->second;

                    m_FreeSizes.erase(It->second);
                    It = m_FreeOffsets.erase(It);
                }
            }
            if (It != m_FreeOffsets.begin())
            {
                --It;
                if (It->first + It->second == Hndl.Offset)
                {
                    Hndl.Offset = It->first;
                    Hndl.Size += It->second;

                    m_FreeSizes.erase(It->second);
                    m_FreeOffsets.erase(It);
                }
            }
            auto Offset = m_FreeOffsets.emplace(Hndl.Offset, Hndl.Size);
            m_FreeSizes.emplace(Hndl.Size, Offset.first);
            Hndl.Size = 0;
        }

    private:
        using OffsetMapType = std::map<size_t, size_t>;
        using SizeMapType   = std::map<size_t, OffsetMapType::const_iterator>;

        OffsetMapType m_FreeOffsets;
        SizeMapType   m_FreeSizes;
    };
} // namespace Neon::Allocator