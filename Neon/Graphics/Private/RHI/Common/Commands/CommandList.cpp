#include <GraphicsPCH.hpp>
#include <RHI/Commands/List.hpp>
#include <RHI/GlobalDescriptors.hpp>

namespace Neon::RHI
{
    void ICommandList::ClearUavFloat(
        IGpuResource*       Resource,
        CpuDescriptorHandle CpuUavHandle,
        const Vector4&      Value)
    {
        auto FrameDescriptor  = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
        auto StagedDescriptor = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView);

        auto FrameUav = FrameDescriptor->Allocate(1);
        FrameUav->Copy(FrameUav.Offset, { .Descriptor = CpuUavHandle, .CopySize = 1 });

        ClearUavFloat(Resource, FrameUav.GetGpuHandle(), CpuUavHandle, Value);
    }

    void ICommandList::ClearUavUInt(
        IGpuResource*       Resource,
        CpuDescriptorHandle CpuUavHandle,
        const Vector4U&     Value)
    {
        auto FrameDescriptor  = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
        auto StagedDescriptor = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView);

        auto FrameUav = FrameDescriptor->Allocate(1);
        FrameUav->Copy(FrameUav.Offset, { .Descriptor = CpuUavHandle, .CopySize = 1 });

        ClearUavUInt(Resource, FrameUav.GetGpuHandle(), CpuUavHandle, Value);
    }
} // namespace Neon::RHI