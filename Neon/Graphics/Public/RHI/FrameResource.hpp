#pragma once

#include <Core/Neon.hpp>
#include <RHI/Device.hpp>
#include <boost/container/small_vector.hpp>

namespace Neon::RHI
{
    struct EmptyFrameData
    {
    };

    template<typename _Ty>
    class FrameResource
    {
        using SmallVector = boost::container::small_vector<_Ty, 3>;

    public:
        FrameResource(
            EmptyFrameData)
        {
        }

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
            for (uint32_t i = 0; i < RHI::IRenderDevice::Get()->GetFrameCount(); i++)
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

    private:
        SmallVector m_Resources;
    };
} // namespace Neon::RHI