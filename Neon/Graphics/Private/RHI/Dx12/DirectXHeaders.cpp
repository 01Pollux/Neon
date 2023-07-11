#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

#include <comdef.h>

namespace Neon::RHI
{
    static const char* DredAllocationType(
        D3D12_DRED_ALLOCATION_TYPE Type)
    {
        switch (Type)
        {
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE:
            return "Command Queue";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_ALLOCATOR:
            return "Command Allocator";
        case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_STATE:
            return "Pipeline State";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_LIST:
            return "Command List";
        case D3D12_DRED_ALLOCATION_TYPE_FENCE:
            return "Fence";
        case D3D12_DRED_ALLOCATION_TYPE_DESCRIPTOR_HEAP:
            return "Descriptor Heap";
        case D3D12_DRED_ALLOCATION_TYPE_HEAP:
            return "Heap";
        case D3D12_DRED_ALLOCATION_TYPE_QUERY_HEAP:
            return "Query Heap";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_SIGNATURE:
            return "Command Signature";
        case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_LIBRARY:
            return "Pipeline Library";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER:
            return "Video Decoder";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_PROCESSOR:
            return "Video Processor";
        case D3D12_DRED_ALLOCATION_TYPE_RESOURCE:
            return "Resource";
        case D3D12_DRED_ALLOCATION_TYPE_PASS:
            return "Pass";
        case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSION:
            return "Crypto Session";
        case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSIONPOLICY:
            return "Crypto Session Policy";
        case D3D12_DRED_ALLOCATION_TYPE_PROTECTEDRESOURCESESSION:
            return "Protected Resource Session";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER_HEAP:
            return "Video Decoder Heap";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_POOL:
            return "Command Pool";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_RECORDER:
            return "Command Recorder";
        case D3D12_DRED_ALLOCATION_TYPE_STATE_OBJECT:
            return "State Object";
        case D3D12_DRED_ALLOCATION_TYPE_METACOMMAND:
            return "Meta Command";
        case D3D12_DRED_ALLOCATION_TYPE_SCHEDULINGGROUP:
            return "Scheduling Group";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_ESTIMATOR:
            return "Video Motion Estimator";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_VECTOR_HEAP:
            return "Video Motion Vector Heap";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_EXTENSION_COMMAND:
            return "Video Extension Command";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER:
            return "Video Encoder";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER_HEAP:
            return "Video Encoder Heap";
        case D3D12_DRED_ALLOCATION_TYPE_INVALID:
        default:
            return "Invalid";
        }
    }

    //

    static const char* DredCommandOp(
        D3D12_AUTO_BREADCRUMB_OP Op)
    {
        switch (Op)
        {
        case D3D12_AUTO_BREADCRUMB_OP_SETMARKER:
            return "SetMarker";
        case D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT:
            return "BeginEvent";
        case D3D12_AUTO_BREADCRUMB_OP_ENDEVENT:
            return "EndEvent";
        case D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED:
            return "DrawInstanced";
        case D3D12_AUTO_BREADCRUMB_OP_DRAWINDEXEDINSTANCED:
            return "DrawIndexedInstanced";
        case D3D12_AUTO_BREADCRUMB_OP_EXECUTEINDIRECT:
            return "ExecuteIndirect";
        case D3D12_AUTO_BREADCRUMB_OP_DISPATCH:
            return "Dispatch";
        case D3D12_AUTO_BREADCRUMB_OP_COPYBUFFERREGION:
            return "CopyBufferRegion";
        case D3D12_AUTO_BREADCRUMB_OP_COPYTEXTUREREGION:
            return "CopyTextureRegion";
        case D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE:
            return "CopyResource";
        case D3D12_AUTO_BREADCRUMB_OP_COPYTILES:
            return "CopyTiles";
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCE:
            return "ResolveSubresource";
        case D3D12_AUTO_BREADCRUMB_OP_CLEARRENDERTARGETVIEW:
            return "ClearRenderTargetView";
        case D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW:
            return "ClearUnorderedAccessView";
        case D3D12_AUTO_BREADCRUMB_OP_CLEARDEPTHSTENCILVIEW:
            return "ClearDepthStencilView";
        case D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER:
            return "ResourceBarrier";
        case D3D12_AUTO_BREADCRUMB_OP_EXECUTEBUNDLE:
            return "ExecuteBundle";
        case D3D12_AUTO_BREADCRUMB_OP_PRESENT:
            return "Present";
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVEQUERYDATA:
            return "ResolveQueryData";
        case D3D12_AUTO_BREADCRUMB_OP_BEGINSUBMISSION:
            return "BeginSubmission";
        case D3D12_AUTO_BREADCRUMB_OP_ENDSUBMISSION:
            return "EndSubmission";
        case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME:
            return "DecodeFrame";
        case D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES:
            return "ProcessFrames";
        case D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT:
            return "AtomicCopyBufferUINT";
        case D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT64:
            return "AtomicCopyBufferUINT64";
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCEREGION:
            return "ResolveSubresourceRegion";
        case D3D12_AUTO_BREADCRUMB_OP_WRITEBUFFERIMMEDIATE:
            return "WriteBufferImmediate";
        case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME1:
            return "DecodeFrame1";
        case D3D12_AUTO_BREADCRUMB_OP_SETPROTECTEDRESOURCESESSION:
            return "SetProtectedResourceSession";
        case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME2:
            return "DecodeFrame2";
        case D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES1:
            return "ProcessFrames1";
        case D3D12_AUTO_BREADCRUMB_OP_BUILDRAYTRACINGACCELERATIONSTRUCTURE:
            return "BuildRaytracingAccelerationStructure";
        case D3D12_AUTO_BREADCRUMB_OP_EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO:
            return "EmitRaytracingAccelerationStructurePostBuildInfo";
        case D3D12_AUTO_BREADCRUMB_OP_COPYRAYTRACINGACCELERATIONSTRUCTURE:
            return "CopyRaytracingAccelerationStructure";
        case D3D12_AUTO_BREADCRUMB_OP_DISPATCHRAYS:
            return "DispatchRays";
        case D3D12_AUTO_BREADCRUMB_OP_INITIALIZEMETACOMMAND:
            return "InitializeMetaCommand";
        case D3D12_AUTO_BREADCRUMB_OP_EXECUTEMETACOMMAND:
            return "ExecuteMetaCommand";
        case D3D12_AUTO_BREADCRUMB_OP_ESTIMATEMOTION:
            return "EstimateMotion";
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVEMOTIONVECTORHEAP:
            return "ResolveMotionVectorHeap";
        case D3D12_AUTO_BREADCRUMB_OP_SETPIPELINESTATE1:
            return "SetPipelineState1";
        case D3D12_AUTO_BREADCRUMB_OP_INITIALIZEEXTENSIONCOMMAND:
            return "InitializeExtensionCommand";
        case D3D12_AUTO_BREADCRUMB_OP_EXECUTEEXTENSIONCOMMAND:
            return "ExecuteExtensionCommand";
        case D3D12_AUTO_BREADCRUMB_OP_DISPATCHMESH:
            return "DispatchMesh";
        case D3D12_AUTO_BREADCRUMB_OP_ENCODEFRAME:
            return "EncodeFrame";
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVEENCODEROUTPUTMETADATA:
            return "ResolveEncoderOutputMetadata";
        case D3D12_AUTO_BREADCRUMB_OP_BARRIER:
            return "Barrier";
        default:
            return "Unknown";
        }
    }

    //

    void ThrowIfFailed(
        HRESULT Result)
    {
        if (FAILED(Result))
        {
            const char* Message;
            StringU8    Description;

            if (Result == DXGI_ERROR_DEVICE_REMOVED)
            {
                std::stringstream ErrorBuffer;

                auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

                ErrorBuffer << ("Dx12 device removed, Dumping DRED information:\n");

                WinAPI::ComPtr<ID3D12DeviceRemovedExtendedData> Dred;
                if (SUCCEEDED(Dx12Device->QueryInterface(IID_PPV_ARGS(&Dred))))
                {
                    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput;
                    D3D12_DRED_PAGE_FAULT_OUTPUT       DredPageFaultOutput;

                    if (SUCCEEDED(Dred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput)))
                    {
                        ErrorBuffer << ("DRED: Dumping breadcrumbs:\n");

                        uint32_t CommandLists = 0;
                        auto     Node         = DredAutoBreadcrumbsOutput.pHeadAutoBreadcrumbNode;
                        while (Node && Node->pLastBreadcrumbValue)
                        {
                            uint32_t LastValue = *Node->pLastBreadcrumbValue;
                            if (LastValue != Node->BreadcrumbCount && LastValue != 0)
                            {
                                ErrorBuffer << StringUtils::Format(
                                    "DRED: Commandlist \"{}\" on CommandQueue \"{}\", {} completed of {}\n",
                                    Node->pCommandListDebugNameA ? Node->pCommandListDebugNameA : "Unnamed CommandList",
                                    Node->pCommandQueueDebugNameA ? Node->pCommandQueueDebugNameA : "Unnamed CommandQueue",
                                    LastValue,
                                    Node->BreadcrumbCount);

                                CommandLists++;

                                int32_t FirstOp = std::max(int(LastValue - 100), 0);
                                int32_t LastOp  = std::min(int(LastValue + 20), int(Node->BreadcrumbCount - 1));

                                for (int32_t Op : std::ranges::iota_view(FirstOp, LastOp))
                                {
                                    auto History = &Node->pCommandHistory[Op];
                                    ErrorBuffer << StringUtils::Format("\t[{}] - {}\n", Op, DredCommandOp(Node->pCommandHistory[Op]));
                                }
                            }

                            Node = Node->pNext;
                        }
                        if (!CommandLists)
                        {
                            ErrorBuffer << StringUtils::Format("DRED: No command list found with active outstanding operations (all finished or not started yet).\n");
                        }
                    }
                    else
                    {
                        ErrorBuffer << StringUtils::Format("DRED: No breadcrumbs found.\n");
                    }

                    if (SUCCEEDED(Dred->GetPageFaultAllocationOutput(&DredPageFaultOutput)) && DredPageFaultOutput.PageFaultVA)
                    {
                        ErrorBuffer << StringUtils::Format("DRED: Page fault at address 0x{:X}\n", DredPageFaultOutput.PageFaultVA);
                        ErrorBuffer << StringUtils::Format("DRED: Dumping allocations:\n");

                        auto Node = DredPageFaultOutput.pHeadExistingAllocationNode;
                        while (Node)
                        {
                            ErrorBuffer << StringUtils::Format(
                                "DRED: Allocation \"{}\" of type {}\n",
                                Node->ObjectNameA ? Node->ObjectNameA : "Unnamed Allocation",
                                DredAllocationType(Node->AllocationType));

                            Node = Node->pNext;
                        }

                        ErrorBuffer << StringUtils::Format("DRED: Dumping freed allocations:\n");
                        Node = DredPageFaultOutput.pHeadRecentFreedAllocationNode;
                        while (Node)
                        {
                            ErrorBuffer << StringUtils::Format(
                                Node->ObjectNameA ? Node->ObjectNameA : "Unnamed Allocation",
                                Node->ObjectNameA,
                                DredAllocationType(Node->AllocationType));

                            Node = Node->pNext;
                        }
                    }
                    else
                    {
                        ErrorBuffer << StringUtils::Format("DRED: No page fault allocations found.");
                    }
                }

                NEON_FATAL(ErrorBuffer.str());
            }
            else
            {
                Message = "Bad HRESULT returned. (Code: 0x{:X} ## {})";
            }

            {
                _com_error Error(Result);
                Description = StringUtils::Transform<StringU8>(Error.ErrorMessage());
            }

            NEON_ASSERT(SUCCEEDED(Result), StringUtils::Format(Message, uint32_t(Result), Description));
        }
    }
} // namespace Neon::RHI