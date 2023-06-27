#include <GraphicsPCH.hpp>
#include <RHI/Resource/MappedBuffer.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    using RawLayout    = Structured::RawLayout;
    using CookedLayout = Structured::CookedLayout;

    GPUBuffer::GPUBuffer(
        const CookedLayout* Layout,
        const Ptr<IBuffer>& Buffer,
        Type                BufferType) :
        m_Layout(Layout),
        m_Buffer(Buffer),
        m_BufferType(BufferType)
    {
        Map();
    }

    GPUBuffer::GPUBuffer(
        const CookedLayout* Layout,
        const Desc&         BufferDesc) :
        m_Layout(Layout),
        m_BufferType(BufferDesc.BufferType)
    {
        switch (m_BufferType)
        {
        case GPUBuffer::Type::Default:
            m_Buffer.reset(IBuffer::Create(BufferDesc.Swapchain, BufferDesc));
            break;
        case GPUBuffer::Type::Upload:
            m_Buffer.reset(IUploadBuffer::Create(BufferDesc.Swapchain, BufferDesc));
            break;
        case GPUBuffer::Type::Readback:
            m_Buffer.reset(IReadbackBuffer::Create(BufferDesc.Swapchain, BufferDesc));
            break;
        default:
            break;
        }
        Map();
    }

    GPUBuffer::GPUBuffer(
        const RawLayout& Layout,
        const Desc&      BufferDesc) :
        GPUBuffer(
            Layout.Cook(true),
            BufferDesc)
    {
    }

    GPUBuffer::~GPUBuffer()
    {
        Unmap();
    }

    void GPUBuffer::Map()
    {
        if (m_Buffer)
        {
            switch (m_BufferType)
            {
            case GPUBuffer::Type::Upload:
                m_MappedData = dynamic_cast<IUploadBuffer*>(m_Buffer.get())->Map();
                break;
            case GPUBuffer::Type::Readback:
                m_MappedData = dynamic_cast<IReadbackBuffer*>(m_Buffer.get())->Map();
                break;
            }
        }
    }

    void GPUBuffer::Unmap()
    {
        if (m_Buffer)
        {
            switch (m_BufferType)
            {
            case GPUBuffer::Type::Upload:
                dynamic_cast<IUploadBuffer*>(m_Buffer.get())->Unmap();
                break;
            case GPUBuffer::Type::Readback:
                dynamic_cast<IReadbackBuffer*>(m_Buffer.get())->Unmap();
                break;
            }
        }
    }
} // namespace Neon::RHI
