#include <EnginePCH.hpp>

#include <Math/Frustum.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/LightCullPass.hpp>
#include <RenderGraph/Passes/DepthPrepass.hpp>

#include <RHI/Shaders/GridFrustumGen.hpp>
#include <RHI/Shaders/LightCull.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/Resource/State.hpp>

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
        LightIndexList_Opaque,
        LightIndexList_Transaprent,
        LightGrid_Opaque,
        LightGrid_Transaprent
    };

    //

    LightCullPass::LightCullPass() :
        RenderPass("LightCullPass")
    {
        RHI::Shaders::GridFrustumGenShader GridFrustumGenShader;
        RHI::Shaders::LightCullShader      LightCullShader;

        m_GridFrustumRS =
            RHI::RootSignatureBuilder(STR("GridFrustumGen::RootSignature"))
                .Add32BitConstants<Size2I>("c_DispatchConstants", 0, 1)
                .AddConstantBufferView("_FrameConstant", 0, 0, RHI::ShaderVisibility::All)
                .AddDescriptorTable(
                    "c_Frustum",
                    RHI::RootDescriptorTable()
                        .AddUavRange("", 0, 1, 1),
                    RHI::ShaderVisibility::All)
                .ComputeOnly()
                .AddStandardSamplers()
                .Build();

        m_LightCullRS =
            RHI::RootSignatureBuilder(STR("LightCull::RootSignature"))
                .Add32BitConstants<LightInfo>("c_LightInfo", 0, 1)
                .AddConstantBufferView("_FrameConstant", 0, 0, RHI::ShaderVisibility::All)
                .AddDescriptorTable(
                    "c_ResourcesTable",
                    RHI::RootDescriptorTable()
                        .AddSrvRangeAt("c_DepthBuffer", 0, 1, 1, uint32_t(LightCullRS_Resources::DepthBuffer))
                        .AddSrvRangeAt("c_FrustumGrid", 1, 1, 1, uint32_t(LightCullRS_Resources::FrustumGrid))
                        .AddSrvRangeAt("c_Lights", 2, 1, 1, uint32_t(LightCullRS_Resources::Lights))
                        .AddUavRangeAt("c_LightIndexList_Opaque", 0, 1, 1, uint32_t(LightCullRS_Resources::LightIndexList_Opaque))
                        .AddUavRangeAt("c_LightIndexList_Transparent", 1, 1, 1, uint32_t(LightCullRS_Resources::LightIndexList_Transaprent))
                        .AddUavRangeAt("c_LightGrid_Opaque", 2, 1, 1, uint32_t(LightCullRS_Resources::LightGrid_Opaque))
                        .AddUavRangeAt("c_LightGrid_Transparent", 3, 1, 1, uint32_t(LightCullRS_Resources::LightGrid_Transaprent)),
                    RHI::ShaderVisibility::All)
                .ComputeOnly()
                .AddStandardSamplers()
                .Build();

        m_GridFrustumPSO =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_GridFrustumRS,
                .ComputeShader = GridFrustumGenShader->LoadShader({ .Stage = RHI::ShaderStage::Compute })
            }
                .Build();

        m_LightCullPSO =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_LightCullRS,
                .ComputeShader = LightCullShader->LoadShader({ .Stage = RHI::ShaderStage::Compute })
            }
                .Build();
    }

    LightCullPass::~LightCullPass()
    {
        if (m_GridFrustumViews)
        {
            RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Free(m_GridFrustumViews);
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

        Size2I GridCount = GetGroupCount(OutputSize);

        RecreateGridFrustum(GridCount);
        RecreateLightGrid(GridCount);
        UpdateResources(Storage, GridCount);
    }

    void LightCullPass::DispatchTyped(
        GraphStorage&           Storage,
        RHI::ComputeCommandList CommandList)
    {
        RecreateGridFrustumIfNeeded(Storage, CommandList);
        DispatchLightCull(Storage, CommandList);
    }

    //

    void LightCullPass::DispatchLightCull(
        GraphStorage&           Storage,
        RHI::ComputeCommandList CommandList)
    {
        CommandList.SetRootSignature(m_LightCullRS);
        CommandList.SetPipelineState(m_LightCullPSO);
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

    void LightCullPass::RecreateGridFrustumIfNeeded(
        GraphStorage&           Storage,
        RHI::ComputeCommandList CommandList)
    {
        auto OutputSize = Storage.GetOutputImageSize();
        if (m_GridSize == OutputSize) [[likely]]
        {
            return;
        }

        m_GridSize = OutputSize;
        DispatchGridFrustum(Storage, CommandList, GetGroupCount(OutputSize));
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
            { .Name = STR("LightCull::GridFrustum") }));

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
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList,
        const Size2I&           GridCount)
    {
        auto StateManager = RHI::IResourceStateManager::Get();

        //

        CommandList.SetPipelineState(m_GridFrustumPSO);
        CommandList.SetRootSignature(m_GridFrustumRS);

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

        StateManager->TransitionResource(m_GridFrustum.get(), RHI::EResourceState::UnorderedAccess);
        StateManager->FlushBarriers(CommandList);

        CommandList.Dispatch(
            m_GridSize.x,
            m_GridSize.y);

        StateManager->TransitionResource(m_GridFrustum.get(), RHI::EResourceState::NonPixelShaderResource);
        StateManager->FlushBarriers(CommandList);
    }

    //

    void LightCullPass::RecreateLightGrid(
        const Size2I& GridCount)
    {
        uint32_t BufferCount           = GridCount.x * GridCount.y;
        size_t   SizeInBytesForIndices = sizeof(uint32_t) * (BufferCount * MaxOverlappingLightsPerTile + 1);

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

        OpaqueList.Set(m_LightIndexList_Opaque, false);
        TransparentList.Set(m_LightIndexList_Transparent, false);

        uint32_t BufferCount = GridCount.x * GridCount.y * MaxOverlappingLightsPerTile;
        for (auto& OpaqueView : OpaqueList.GetViews())
        {
            if (auto UavDesc = std::get_if<RHI::UAVDescOpt>(&OpaqueView.second.Desc))
            {
                *UavDesc =
                    RHI::UAVDesc{
                        .View = RHI::UAVDesc::Buffer{
                            .FirstElement = 1,
                            .Count        = BufferCount,
                            .SizeOfStruct = sizeof(uint32_t) }
                    };
            }
        }
        for (auto& TransparentView : TransparentList.GetViews())
        {
            if (auto UavDesc = std::get_if<RHI::UAVDescOpt>(&TransparentView.second.Desc))
            {
                *UavDesc =
                    RHI::UAVDesc{
                        .View = RHI::UAVDesc::Buffer{
                            .FirstElement = 1,
                            .Count        = BufferCount,
                            .SizeOfStruct = sizeof(uint32_t) }
                    };
            }
        }
    }
} // namespace Neon::RG