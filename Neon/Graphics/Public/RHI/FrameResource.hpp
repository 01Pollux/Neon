#pragma once

#include <Core/Neon.hpp>
#include <RHI/Device.hpp>
#include <boost/container/small_vector.hpp>

namespace Neon::RHI
{
    template<typename _Ty>
    class FrameResource
    {
        using SmallVector = boost::container::small_vector<_Ty, 3>;

    public:
        template<typename... _Args>
        FrameResource(
            _Args&&... Args) noexcept
        {
            Initialize(std::forward<_Args>(Args)...);
        }

        /// <summary>
        /// Inialize the frame resource
        /// </summary>
        template<typename... _Args>
        void Initialize(
            _Args&&... Args)
        {
            Reset();
            m_Resources.reserve(Size());
            for (uint32_t i = 0; i < Size(); i++)
            {
                m_Resources.emplace_back(std::forward<_Args>(Args)...);
            }
        }

        /// <summary>
        /// Reset the frame resource
        /// </summary>
        void Reset()
        {
            m_Resources.clear();
        }

        /// <summary>
        /// Get the current frame resource
        /// </summary>
        [[nodiscard]] _Ty& Get() noexcept
        {
            return m_Resources[RHI::IRenderDevice::Get()->GetFrameIndex()];
        }

        /// <summary>
        /// Get the current frame resource
        /// </summary>
        [[nodiscard]] const _Ty& Get() const noexcept
        {
            return m_Resources[RHI::IRenderDevice::Get()->GetFrameIndex()];
        }

        /// <summary>
        /// Get the current frame resource
        /// </summary>
        [[nodiscard]] _Ty& GetAt(
            size_t Index) noexcept
        {
            return m_Resources[Index];
        }

        /// <summary>
        /// Get the current frame resource
        /// </summary>
        [[nodiscard]] const _Ty& GetAt(
            size_t Index) const noexcept
        {
            return m_Resources[Index];
        }

        /// <summary>
        /// Get the current frame resource
        /// </summary>
        [[nodiscard]] _Ty* operator->() noexcept
        {
            return std::addressof(Get());
        }

        /// <summary>
        /// Get the current frame resource
        /// </summary>
        [[nodiscard]] const _Ty* operator->() const noexcept
        {
            return std::addressof(Get());
        }

        /// <summary>
        /// Get the number of frames
        /// </summary>
        [[nodiscard]] uint32_t Size() const noexcept
        {
            return RHI::IRenderDevice::Get()->GetFrameCount();
        }

    private:
        SmallVector m_Resources;
    };
} // namespace Neon::RHI