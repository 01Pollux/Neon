#include <EnginePCH.hpp>

#include <Renderer/Material/Builder.hpp>
#include <Private/Renderer/Material/Material.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>

#include <Log/Logger.hpp>

namespace ranges = std::ranges;
namespace views  = std::views;

namespace Neon::Renderer
{
    Ptr<IMaterial> IMaterial::Create(
        RHI::ISwapchain*                     Swapchain,
        const GenericMaterialBuilder<false>& Builder)
    {
        uint32_t LocalResourceDescriptorSize = 0,
                 LocalSamplerDescriptorSize  = 0;

        Ptr<Material> Mat{ NEON_NEW Material(Swapchain, Builder, LocalResourceDescriptorSize, LocalSamplerDescriptorSize) };
        Mat->CreateDefaultInstance(LocalResourceDescriptorSize, LocalSamplerDescriptorSize);
        return Mat;
    }

    Ptr<IMaterial> IMaterial::Create(
        RHI::ISwapchain*                    Swapchain,
        const GenericMaterialBuilder<true>& Builder)
    {
        uint32_t LocalResourceDescriptorSize = 0,
                 LocalSamplerDescriptorSize  = 0;

        Ptr<Material> Mat{ NEON_NEW Material(Swapchain, Builder, LocalResourceDescriptorSize, LocalSamplerDescriptorSize) };
        Mat->CreateDefaultInstance(LocalResourceDescriptorSize, LocalSamplerDescriptorSize);
        return Mat;
    }

    //

    static void CreateDescriptorIfNeeded(
        RHI::DescriptorHeapHandle& Descriptor,
        RHI::ISwapchain*           Swapchain,
        RHI::DescriptorType        Type,
        uint32_t                   Count)
    {
        auto Allocator = Swapchain->GetDescriptorHeapManager(Type, false);
        if (Count)
        {
            Descriptor = Allocator->Allocate(Count);
        }
    }

    template<bool _Compute>
    static void Material_CreateDescriptors(
        RHI::ISwapchain*                        Swapchain,
        const GenericMaterialBuilder<_Compute>& Builder,
        Material*                               Mat,
        uint32_t&                               LocaResourceDescriptorSize,
        uint32_t&                               LocaSamplerDescriptorSize)
    {
        RHI::RootSignatureBuilder RootSigBuilder;

        uint32_t TableResourceCount       = 0,
                 TableSharedResourceCount = 0;
        uint32_t TableSamplerCount        = 0,
                 TableSharedSamplerCount  = 0;

        uint32_t RootIndex = 0;

        struct BatchedDescriptorEntry
        {
            Material::LayoutEntry* LayoutEntry;

            ShaderBinding Binding;
            uint32_t      Size;

            auto operator<=>(const BatchedDescriptorEntry& Other) const noexcept
            {
                return Binding <=> Other.Binding;
            }
        };

        std::map<
            RHI::ShaderVisibility,
            std::map<MaterialVarType,
                     std::set<BatchedDescriptorEntry>>>
            BatchedDescriptorEntries[2];

        uint32_t RootParamIndex = 0;

        auto& VarMap = Builder.VarMap();

        VarMap.ForEachVariable(
            [Mat,
             &BatchedDescriptorEntries,
             &TableResourceCount,
             &TableSharedResourceCount,
             &TableSamplerCount,
             &TableSharedSamplerCount](
                const MaterialVariableMap::View& View) mutable
            {
                switch (View.Type())
                {
                case MaterialVarType::Buffer:
                case MaterialVarType::Resource:
                case MaterialVarType::RWResource:
                case MaterialVarType::Sampler:
                {
                    size_t BatchIndex = View.Type() == MaterialVarType::Sampler ? 1 : 0;

                    auto& Descriptors = BatchedDescriptorEntries[BatchIndex][View.Visibility()][View.Type()];

                    Material::DescriptorEntry DescriptorEntry{};

                    DescriptorEntry.Descs.resize(View.ArraySize());
                    DescriptorEntry.Resources.resize(View.ArraySize());

                    const bool IsShared = View.Flags().Test(EMaterialVarFlags::Shared);

                    auto& DescriptorCount =
                        View.Type() == MaterialVarType::Sampler
                            ? (IsShared ? TableSharedSamplerCount : TableSamplerCount)
                            : (IsShared ? TableSharedResourceCount : TableResourceCount);

                    DescriptorEntry.Offset = DescriptorCount;
                    DescriptorCount += View.ArraySize();

                    const uint32_t EntrySize = IsShared ? View.ArraySize() : Material::UnboundedTableSize;

                    auto& LayoutEntry    = Mat->m_EntryMap[View.Name()];
                    LayoutEntry.Entry    = std::move(DescriptorEntry);
                    LayoutEntry.IsShared = IsShared;

                    Descriptors.insert(
                        BatchedDescriptorEntry{
                            .LayoutEntry = &LayoutEntry,
                            .Binding     = View.Binding(),
                            .Size        = EntrySize });

                    break;
                }

                default:
                    NEON_ASSERT(false);
                }
            });

        VarMap.ForEachStaticSampler(
            [&RootSigBuilder](
                const RHI::StaticSamplerDesc& Desc)
            {
                RootSigBuilder.AddSampler(Desc);
            });

        // Merge batched descriptors
        for (auto& Entries : BatchedDescriptorEntries)
        {
            for (auto& [Visibility, DescriptorEntries] : Entries)
            {
                RHI::RootDescriptorTable Table;
                for (auto& [Type, Descriptors] : DescriptorEntries)
                {
                    for (auto& [Entry, Binding, Size] : Descriptors)
                    {
                        RHI::MRootDescriptorTableFlags Flags;
                        Flags.Set(RHI::ERootDescriptorTableFlags::Descriptor_Volatile);
                        Flags.Set(RHI::ERootDescriptorTableFlags::Data_Static_While_Execute);

                        switch (Type)
                        {
                        case MaterialVarType::Buffer:
                            Table.AddCbvRange(Binding.Register, Binding.Space, Size, std::move(Flags));
                            break;

                        case MaterialVarType::Resource:
                            Table.AddSrvRange(Binding.Register, Binding.Space, Size, std::move(Flags));
                            break;

                        case MaterialVarType::RWResource:
                            Table.AddUavRange(Binding.Register, Binding.Space, Size, std::move(Flags));
                            break;

                        case MaterialVarType::Sampler:
                            Table.AddSamplerRange(Binding.Register, Binding.Space, Size, std::move(Flags));
                            break;
                        }

                        Entry->RootIndex = RootParamIndex;
                    }
                }
                RootSigBuilder.AddDescriptorTable(std::move(Table), Visibility);
                ++RootParamIndex;
            }
        }

        //

        if constexpr (_Compute)
        {
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyVSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyHSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyDSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyGSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyPSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyAmpAcess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyMeshAccess, true);
        }
        else
        {
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout, !Builder.NoVertexInput());

            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyVSAccess, !Builder.VertexShader().Enabled);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyHSAccess, !Builder.HullShader().Enabled);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyDSAccess, !Builder.DomainShader().Enabled);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyGSAccess, !Builder.GeometryShader().Enabled);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyPSAccess, !Builder.PixelShader().Enabled);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyAmpAcess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyMeshAccess, true);
        }

        Mat->m_RootSignature = RHI::IRootSignature::Create(RootSigBuilder);

        //

        CreateDescriptorIfNeeded(Mat->m_SharedResourceDescriptor, Swapchain, RHI::DescriptorType::ResourceView, TableSharedResourceCount);
        CreateDescriptorIfNeeded(Mat->m_SharedSamplerDescriptor, Swapchain, RHI::DescriptorType::Sampler, TableSharedSamplerCount);

        LocaResourceDescriptorSize = TableResourceCount;
        LocaSamplerDescriptorSize  = TableSamplerCount;
    }

    //

    template<bool _Compute>
    void Material_CreatePipelineState(
        const GenericMaterialBuilder<_Compute>& Builder,
        Material*                               Mat)
    {
        auto LoadShader = [&Builder](const auto& ModuleHandle, RHI::ShaderStage Stage)
        {
            Ptr<RHI::IShader> Shader;
            if (ModuleHandle.Enabled)
            {
                auto& ShaderLib = Builder.ShaderLibrary();
                auto  Module    = ShaderLib->LoadModule(ModuleHandle.ModuleId);
                if (Module)
                {
                    Shader = Module->LoadStage(Stage, ModuleHandle.Flags, ModuleHandle.Profile, ModuleHandle.Macros);
                }
                else
                {
                    NEON_WARNING("Material", "Failed to load shader module: {:X}", ModuleHandle.ModuleId);
                }
            }
            return Shader;
        };

        if constexpr (!_Compute)
        {
            RHI::PipelineStateBuilderG PipelineDesc{
                .RootSignature = Mat->m_RootSignature,

                .VertexShader   = LoadShader(Builder.VertexShader(), RHI::ShaderStage::Vertex),
                .PixelShader    = LoadShader(Builder.PixelShader(), RHI::ShaderStage::Pixel),
                .GeometryShader = LoadShader(Builder.GeometryShader(), RHI::ShaderStage::Geometry),
                .HullShader     = LoadShader(Builder.HullShader(), RHI::ShaderStage::Hull),
                .DomainShader   = LoadShader(Builder.DomainShader(), RHI::ShaderStage::Domain),

                .Blend        = Builder.Blend(),
                .Rasterizer   = Builder.Rasterizer(),
                .DepthStencil = Builder.DepthStencil(),

                .RTFormats = Builder.RenderTargets() |
                             views::transform([](auto& RenderTarget)
                                              { return RenderTarget.second.second; }) |
                             ranges::to<std::vector>(),

                .SampleMask    = Builder.SampleMask(),
                .SampleCount   = Builder.SampleCount(),
                .SampleQuality = Builder.SampleQuality(),

                .StripCut = Builder.StripCut(),
                .DSFormat = Builder.DepthStencilFormat()
            };

            if (auto& InputLayout = Builder.InputLayout();
                !InputLayout.has_value() && !Builder.NoVertexInput())
            {
                PipelineDesc.VertexShader->CreateInputLayout(PipelineDesc.Input);
            }
            else
            {
                PipelineDesc.Input = InputLayout.value();
            }

            using PipelineTopology = RHI::PipelineStateBuilderG::PrimitiveTopology;
            switch (Builder.Topology())
            {
            case RHI::PrimitiveTopology::PointList:
                PipelineDesc.Topology = PipelineTopology::Point;
                break;
            case RHI::PrimitiveTopology::LineList:
            case RHI::PrimitiveTopology::LineStrip:
            case RHI::PrimitiveTopology::LineList_Adj:
            case RHI::PrimitiveTopology::LineStrip_Adj:
                PipelineDesc.Topology = PipelineTopology::Line;
                break;
            case RHI::PrimitiveTopology::TriangleList:
            case RHI::PrimitiveTopology::TriangleStrip:
            case RHI::PrimitiveTopology::TriangleList_Adj:
            case RHI::PrimitiveTopology::TriangleStrip_Adj:
                PipelineDesc.Topology = PipelineTopology::Triangle;
                break;
            case RHI::PrimitiveTopology::PatchList_1:
            case RHI::PrimitiveTopology::PatchList_2:
            case RHI::PrimitiveTopology::PatchList_3:
            case RHI::PrimitiveTopology::PatchList_4:
            case RHI::PrimitiveTopology::PatchList_5:
            case RHI::PrimitiveTopology::PatchList_6:
            case RHI::PrimitiveTopology::PatchList_7:
            case RHI::PrimitiveTopology::PatchList_8:
            case RHI::PrimitiveTopology::PatchList_9:
            case RHI::PrimitiveTopology::PatchList_10:
            case RHI::PrimitiveTopology::PatchList_11:
            case RHI::PrimitiveTopology::PatchList_12:
            case RHI::PrimitiveTopology::PatchList_13:
            case RHI::PrimitiveTopology::PatchList_14:
            case RHI::PrimitiveTopology::PatchList_15:
            case RHI::PrimitiveTopology::PatchList_16:
            case RHI::PrimitiveTopology::PatchList_17:
            case RHI::PrimitiveTopology::PatchList_18:
            case RHI::PrimitiveTopology::PatchList_19:
            case RHI::PrimitiveTopology::PatchList_20:
            case RHI::PrimitiveTopology::PatchList_21:
            case RHI::PrimitiveTopology::PatchList_22:
            case RHI::PrimitiveTopology::PatchList_23:
            case RHI::PrimitiveTopology::PatchList_24:
            case RHI::PrimitiveTopology::PatchList_25:
            case RHI::PrimitiveTopology::PatchList_26:
            case RHI::PrimitiveTopology::PatchList_27:
            case RHI::PrimitiveTopology::PatchList_28:
            case RHI::PrimitiveTopology::PatchList_29:
            case RHI::PrimitiveTopology::PatchList_30:
            case RHI::PrimitiveTopology::PatchList_31:
            case RHI::PrimitiveTopology::PatchList_32:
                PipelineDesc.Topology = PipelineTopology::Patch;
                break;
            default:
                std::unreachable();
            }

            Mat->m_PipelineState = RHI::IPipelineState::Create(PipelineDesc);
        }
        else
        {
            RHI::PipelineStateBuilderC PipelineDesc{
                .RootSignature = Mat->m_RootSignature,
                .ComputeShader = LoadShader(Builder.ComputeShader(), RHI::ShaderStage::Compute)
            };

            Mat->m_PipelineState = RHI::IPipelineState::Create(PipelineDesc);
        }
    }

    //

    Material::Material(
        RHI::ISwapchain*             Swapchain,
        const RenderMaterialBuilder& Builder,
        uint32_t&                    LocalResourceDescriptorSize,
        uint32_t&                    LocalSamplerDescriptorSize) :
        m_Swapchain(Swapchain)
    {
        Material_CreateDescriptors(
            Swapchain,
            Builder,
            this,
            LocalResourceDescriptorSize,
            LocalSamplerDescriptorSize);

        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        RHI::ISwapchain*              Swapchain,
        const ComputeMaterialBuilder& Builder,
        uint32_t&                     LocalResourceDescriptorSize,
        uint32_t&                     LocalSamplerDescriptorSize)
    {
        Material_CreateDescriptors(
            Swapchain,
            Builder,
            this,
            LocalResourceDescriptorSize,
            LocalSamplerDescriptorSize);

        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::~Material()
    {
        if (m_SharedResourceDescriptor)
        {
            m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::ResourceView, false)->Free(m_SharedResourceDescriptor);
        }
        if (m_SharedSamplerDescriptor)
        {
            m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::Sampler, false)->Free(m_SharedSamplerDescriptor);
        }
    }

    Ptr<MaterialInstance> Material::CreateInstance()
    {
        return m_DefaultInstace->CreateInstance();
    }

    void Material::CreateDefaultInstance(
        uint32_t LocalResourceDescriptorSize,
        uint32_t LocalSamplerDescriptorSize)
    {
        NEON_ASSERT(!m_DefaultInstace);
        m_DefaultInstace.reset(NEON_NEW MaterialInstance(
            std::static_pointer_cast<Material>(shared_from_this()),
            LocalResourceDescriptorSize,
            LocalSamplerDescriptorSize));
    }

    //

    MaterialInstance::MaterialInstance(
        Ptr<Material> Mat,
        uint32_t      LocaResourceDescriptorSize,
        uint32_t      LocaSamplerDescriptorSize) :
        m_Swapchain(Mat->m_Swapchain),
        m_ParentMaterial(std::move(Mat))
    {
        CreateDescriptorIfNeeded(m_ResourceDescriptor, m_Swapchain, RHI::DescriptorType::ResourceView, LocaResourceDescriptorSize);
        CreateDescriptorIfNeeded(m_SamplerDescriptor, m_Swapchain, RHI::DescriptorType::Sampler, LocaSamplerDescriptorSize);
    }

    Ptr<MaterialInstance> MaterialInstance::CreateInstance()
    {
        return std::make_shared<MaterialInstance>(
            GetParentMaterial(),
            m_ResourceDescriptor.Size,
            m_SamplerDescriptor.Size);
    }

    const Ptr<Material>& MaterialInstance::GetParentMaterial() const
    {
        return m_ParentMaterial;
    }

    MaterialInstance::~MaterialInstance()
    {
        if (m_ResourceDescriptor)
        {
            m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::ResourceView, true)->Free(m_ResourceDescriptor);
        }
        if (m_SamplerDescriptor)
        {
            m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::Sampler, true)->Free(m_SamplerDescriptor);
        }
    }
} // namespace Neon::Renderer