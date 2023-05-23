#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

#include <comdef.h>

namespace Neon::RHI
{
    void ThrowIfFailed(
        HRESULT Result)
    {
        if (FAILED(Result))
        {
            const char* Message;
            StringU8    Description;

            if (Result == DXGI_ERROR_DEVICE_REMOVED)
            {
                auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
                Result          = Dx12Device->GetDeviceRemovedReason();

                Message = "Device removed. (Code: 0x{:X} ## {})";
            }
            else
            {
                Message = "Bad HRESULT returned. (Code: 0x{:X} ## {})";
            }

            {
                _com_error Error(Result);
                Description = StringUtils::StringTransform<StringU8>(Error.ErrorMessage());
            }

            NEON_ASSERT(SUCCEEDED(Result), StringUtils::Format(Message, uint32_t(Result), Description));
        }
    }
} // namespace Neon::RHI