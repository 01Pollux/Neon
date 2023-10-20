#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/LightCullPass.hpp>
#include <RenderGraph/Passes/DepthPrepass.hpp>

#include <RHI/Shaders/GridFrustumGen.hpp>
#include <RHI/Shaders/LightCull.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/Resource/State.hpp>
#include <RHI/GlobalDescriptors.hpp>

#include <Geometry/Frustum.hpp>

namespace Neon::RG
{
    enum class GridFrustumRS : uint8_t
    {
        Constants_DispatchConstants,
        Cbv_FrameConstants,
        Table_Frustum
    };

    enum class LightCullRS : uint8_t
    {
        Constants_LightInfo,
        Cbv_FrameConstants,
        Table_Resources
    };

    enum class LightCullRS_Resources : uint8_t
    {
        DepthBuffer,
        FrustumGrid,
        Lights,
        LightIndexList_Counters,
        LightIndexList_Opaque,
        LightIndexList_Transaprent,
        LightGrid_Opaque,
        LightGrid_Transaprent,

        Count
    };

    //

    LightCullPass::LightCullPass() :
        RenderPass("LightCullPass")
    {
        RHI::Shaders::GridFrustumGenShader GridFrustumGenShader;
        RHI::Shaders::LightCullShader      LightCullShader;

        auto GridFrustumRootSig = RHI::IRootSignature::Get(RHI::RSCommon::Type::GridFrustum);
        auto LightCullRootSig   = RHI::IRootSignature::Get(RHI::RSCommon::Type::LightCull);

        m_GridFrustumPSO =
            RHI::PipelineStateBuilderC{
                .RootSignature = GridFrustumRootSig,
                .ComputeShader = GridFrustumGenShader->LoadShader({ .Stage = RHI::ShaderStage::Compute })
            }
                .Build();

        m_LightCullPSO =
            RHI::PipelineStateBuilderC{
                .RootSignature = LightCullRootSig,
                .ComputeShader = LightCullShader->LoadShader({ .Stage = RHI::ShaderStage::Compute })
            }
                .Build();

        //

        m_LightIndexList_Counters.reset(
            RHI::IGpuResource::Create(
                RHI::ResourceDesc::Buffer(
                    sizeof(uint32_t) * 2,
                    BitMask_Or(RHI::EResourceFlags::AllowUnorderedAccess)),
                { .Name         = STR("LightIndexList_Counters"),
                  .InitialState = BitMask_Or(RHI::EResourceState::UnorderedAccess) }));

        auto Descriptor               = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
        m_LightIndexList_CountersView = Descriptor->Allocate(1);

        RHI::UAVDesc UavDesc{
            .View = RHI::UAVDesc::Buffer{
                .Count = 2,
                .Raw   = true },
            .Format = RHI::EResourceFormat::R32_Typeless
        };
        m_LightIndexList_CountersView->CreateUnorderedAccessView(
            m_LightIndexList_CountersView.Offset,
            m_LightIndexList_Counters.get(),
            &UavDesc);
    }

    LightCullPass::~LightCullPass()
    {
        auto Descriptor = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
        if (m_GridFrustumViews)
        {
            Descriptor->Free(m_GridFrustumViews);
            m_GridFrustumViews = {};
        }
        if (m_LightIndexList_CountersView)
        {
            Descriptor->Free(m_LightIndexList_CountersView);
            m_LightIndexList_CountersView = {};
        }
    }

    void LightCullPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        Resolver.ReadTexture(
            DepthPrepass::DepthBuffer.CreateView("LightCull"),
            ResourceReadAccess::NonPixelShader,
            RHI::SRVDesc{
                .View   = RHI::SRVDesc::Texture2D{},
                .Format = RHI::EResourceFormat::R32_Float });

        Resolver.ImportTexture(LightCullPass::LightIndexList_Opaque, nullptr);
        Resolver.ImportTexture(LightCullPass::LightIndexList_Transparent, nullptr);
        Resolver.ImportTexture(LightCullPass::LightGrid_Opaque, nullptr);
        Resolver.ImportTexture(LightCullPass::LightGrid_Transparent, nullptr);

        Resolver.WriteTexture(
            LightCullPass::LightIndexList_Opaque.CreateView("Main"));
        Resolver.WriteTexture(
            LightCullPass::LightIndexList_Transparent.CreateView("Main"));

        Resolver.WriteTexture(
            LightCullPass::LightGrid_Opaque.CreateView("Main"));
        Resolver.WriteTexture(
            LightCullPass::LightGrid_Transparent.CreateView("Main"));
    }

    void LightCullPass::PreDispatch(
        GraphStorage& Storage)
    {
        auto OutputSize = Storage.GetOutputImageSize();
        if (m_GridSize == OutputSize) [[likely]]
        {
            return;
        }

        m_GridSize       = OutputSize;
        Size2I GridCount = GetGroupCount(OutputSize);

        RecreateGridFrustum(GridCount);
        DispatchGridFrustum(Storage, GridCount);
        RecreateLightGrid(GridCount);
        UpdateResources(Storage, GridCount);
    }

    void LightCullPass::DispatchTyped(
        GraphStorage&           Storage,
        RHI::ComputeCommandList CommandList)
    {
        DispatchLightCull(Storage, CommandList);
    }

    //

    void LightCullPass::DispatchLightCull(
        GraphStorage&           Storage,
        RHI::ComputeCommandList CommandList)
    {
        auto& SceneContext = Storage.GetSceneContext();

        auto LightCullRootSig = RHI::IRootSignature::Get(RHI::RSCommon::Type::LightCull);

        CommandList.SetRootSignature(LightCullRootSig);
        CommandList.SetPipelineState(m_LightCullPSO);

        CommandList.SetConstants(uint32_t(LightCullRS::Constants_LightInfo), SceneContext.GetLightsCount());
        CommandList.SetResourceView(RHI::CstResourceViewType::Cbv, uint32_t(LightCullRS::Cbv_FrameConstants), Storage.GetFrameDataHandle());
        {
            auto Descriptor = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(uint32_t(LightCullRS_Resources::Count));

            std::array Sources{
                // DepthBuffer
                RHI::IDescriptorHeap::CopyInfo{
                    .Descriptor = Storage.GetResourceViewHandle(DepthPrepass::DepthBuffer.CreateView("LightCull")),
                    .CopySize   = 1 },
                // FrustumGrid
                RHI::IDescriptorHeap::CopyInfo{
                    .Descriptor = m_GridFrustumViews.GetCpuHandle(1),
                    .CopySize   = 1 },
                // Lights
                RHI::IDescriptorHeap::CopyInfo{
                    .Descriptor = SceneContext.GetLightsResourceView(),
                    .CopySize   = 1 },
                // Light index counters
                RHI::IDescriptorHeap::CopyInfo{
                    .Descriptor = m_LightIndexList_CountersView.GetCpuHandle(),
                    .CopySize   = 1 },
                // Light index list opaque
                RHI::IDescriptorHeap::CopyInfo{
                    .Descriptor = Storage.GetResourceViewHandle(LightCullPass::LightIndexList_Opaque.CreateView("Main")),
                    .CopySize   = 1 },
                // Light index list transparent
                RHI::IDescriptorHeap::CopyInfo{
                    .Descriptor = Storage.GetResourceViewHandle(LightCullPass::LightIndexList_Transparent.CreateView("Main")),
                    .CopySize   = 1 },
                // Light grid opaque
                RHI::IDescriptorHeap::CopyInfo{
                    .Descriptor = Storage.GetResourceViewHandle(LightCullPass::LightGrid_Opaque.CreateView("Main")),
                    .CopySize   = 1 },
                // Light grid transparent
                RHI::IDescriptorHeap::CopyInfo{
                    .Descriptor = Storage.GetResourceViewHandle(LightCullPass::LightGrid_Transparent.CreateView("Main")),
                    .CopySize   = 1 }
            };
            static_assert(Sources.size() == uint32_t(LightCullRS_Resources::Count));

            Descriptor->Copy(
                Descriptor.Offset,
                Sources);

            CommandList.SetDescriptorTable(uint32_t(LightCullRS::Table_Resources), Descriptor.GetGpuHandle());
            CommandList.ClearUavUInt(
                m_LightIndexList_Counters.get(),
                Descriptor.GetGpuHandle(uint32_t(LightCullRS_Resources::LightIndexList_Counters)),
                m_LightIndexList_CountersView.GetCpuHandle());
        }

        CommandList.Dispatch(m_GridSize.x, m_GridSize.y);
    }

    //

    Size2I LightCullPass::GetGroupCount(
        const Size2I& OutputSize) const
    {
        auto& GroupSize = m_GridFrustumPSO->GetComputeGroupSize();

        return Size2I{
            int(Math::DivideByMultiple(OutputSize.x, GroupSize.x)),
            int(Math::DivideByMultiple(OutputSize.y, GroupSize.y))
        };
    }

    //

    void LightCullPass::RecreateGridFrustum(
        const Size2I& GridCount)
    {
        uint32_t BufferCount = GridCount.x * GridCount.y;
        size_t   SizeInBytes = sizeof(Geometry::Frustum) * BufferCount;

        RHI::MResourceFlags Flags;
        Flags.Set(RHI::EResourceFlags::AllowUnorderedAccess);

        auto Descriptor = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);

        if (m_GridFrustumViews)
        {
            Descriptor->Free(m_GridFrustumViews);
        }

        m_GridFrustum.reset(RHI::IGpuResource::Create(
            RHI::ResourceDesc::Buffer(
                SizeInBytes,
                Flags),
            { .Name         = STR("LightCull::GridFrustum"),
              .InitialState = BitMask_Or(RHI::EResourceState::UnorderedAccess) }));

        m_GridFrustumViews = Descriptor->Allocate(2);

        RHI::UAVDesc UavDesc{
            .View = RHI::UAVDesc::Buffer{
                .Count        = BufferCount,
                .SizeOfStruct = sizeof(Geometry::Frustum) }
        };
        m_GridFrustumViews->CreateUnorderedAccessView(
            m_GridFrustumViews.Offset,
            m_GridFrustum.get(),
            &UavDesc);

        RHI::SRVDesc SrvDesc{
            .View = RHI::SRVDesc::Buffer{
                .Count        = BufferCount,
                .SizeOfStruct = sizeof(Geometry::Frustum) }
        };
        m_GridFrustumViews->CreateShaderResourceView(
            m_GridFrustumViews.Offset + 1,
            m_GridFrustum.get(),
            &SrvDesc);
    }

    //

    void LightCullPass::DispatchGridFrustum(
        const GraphStorage& Storage,
        const Size2I&       GridCount)
    {
        auto StateManager = RHI::IResourceStateManager::Get();

        RHI::CommandContext     CommandContext;
        RHI::ComputeCommandList CommandList(CommandContext.Append());

        //
        auto GridFrustumRootSig = RHI::IRootSignature::Get(RHI::RSCommon::Type::GridFrustum);

        CommandList.SetRootSignature(GridFrustumRootSig);
        CommandList.SetPipelineState(m_GridFrustumPSO);

        CommandList.SetConstants(
            uint32_t(GridFrustumRS::Constants_DispatchConstants),
            &GridCount.x,
            Math::DivideByMultiple(sizeof(GridCount), sizeof(uint32_t)));

        CommandList.SetResourceView(
            RHI::CstResourceViewType::Cbv,
            uint32_t(GridFrustumRS::Cbv_FrameConstants),
            Storage.GetFrameDataHandle());

        CommandList.SetDynamicDescriptorTable(
            uint32_t(GridFrustumRS::Table_Frustum),
            m_GridFrustumViews.GetCpuHandle(),
            1,
            false);

        // Flush pending UnorderedAccess transition
        StateManager->FlushBarriers(CommandList);

        CommandList.Dispatch(
            m_GridSize.x,
            m_GridSize.y);

        StateManager->TransitionResource(m_GridFrustum.get(), RHI::EResourceState::NonPixelShaderResource);
    }

    //

    void LightCullPass::RecreateLightGrid(
        const Size2I& GridCount)
    {
        uint32_t BufferCount           = GridCount.x * GridCount.y * MaxOverlappingLightsPerTile;
        size_t   SizeInBytesForIndices = Math::AlignUp(sizeof(uint32_t) * BufferCount, 16);

        RHI::MResourceFlags Flags;
        Flags.Set(RHI::EResourceFlags::AllowUnorderedAccess);

        // Create opaque and transparent index list
        {
            m_LightIndexList_Opaque.reset(RHI::IGpuResource::Create(
                RHI::ResourceDesc::Buffer(
                    SizeInBytesForIndices,
                    Flags),
                { .Name = STR("LightCull::LightIndexList_Opaque") }));

            m_LightIndexList_Transparent.reset(RHI::IGpuResource::Create(
                RHI::ResourceDesc::Buffer(
                    SizeInBytesForIndices,
                    Flags),
                { .Name = STR("LightCull::LightIndexList_Transparent") }));
        }

        // Create opaque and transparent grid
        {
            m_LightGrid_Opaque.reset(RHI::IGpuResource::Create(
                RHI::ResourceDesc::Tex2D(
                    RHI::EResourceFormat::R32G32_UInt,
                    GridCount.x,
                    GridCount.y,
                    1, 1,
                    1, 0,
                    Flags),
                { .Name = STR("LightCull::LightGrid_Opaque") }));

            m_LightGrid_Transparent.reset(RHI::IGpuResource::Create(
                RHI::ResourceDesc::Tex2D(
                    RHI::EResourceFormat::R32G32_UInt,
                    GridCount.x,
                    GridCount.y,
                    1, 1,
                    1, 0,
                    Flags),
                { .Name = STR("LightCull::LightGrid_Transparent") }));
        }
    }

    //

    void LightCullPass::UpdateResources(
        GraphStorage& Storage,
        const Size2I& GridCount)
    {
        auto& OpaqueList      = Storage.GetResourceMut(LightCullPass::LightIndexList_Opaque);
        auto& TransparentList = Storage.GetResourceMut(LightCullPass::LightIndexList_Transparent);
        Storage.GetResourceMut(LightCullPass::LightGrid_Transparent).Set(m_LightGrid_Transparent, false);
        Storage.GetResourceMut(LightCullPass::LightGrid_Opaque).Set(m_LightGrid_Opaque, false);

        uint32_t       BufferCount = GridCount.x * GridCount.y * MaxOverlappingLightsPerTile;
        VariantVisitor Visitor{
            [BufferCount](RHI::UAVDescOpt& UavDesc)
            {
                UavDesc = RHI::UAVDesc{
                    .View = RHI::UAVDesc::Buffer{
                        .Count        = BufferCount,
                        .SizeOfStruct = sizeof(uint32_t) }
                };
            },
            [BufferCount](RHI::SRVDescOpt& SrvDesc)
            {
                SrvDesc = RHI::SRVDesc{
                    .View = RHI::SRVDesc::Buffer{
                        .Count        = BufferCount,
                        .SizeOfStruct = sizeof(uint32_t) }
                };
            },
            [](const auto&) {}
        };

        OpaqueList.Set(m_LightIndexList_Opaque, Visitor, false);
        TransparentList.Set(m_LightIndexList_Transparent, Visitor, false);
    }
} // namespace Neon::RG