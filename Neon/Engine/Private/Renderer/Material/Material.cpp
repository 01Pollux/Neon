#include <EnginePCH.hpp>

#include <Renderer/Material/Builder.hpp>
#include <Private/Renderer/Material/Material.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/GlobalDescriptors.hpp>

#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/Commands/List.hpp>

#include <RHI/Resource/Views/ConstantBuffer.hpp>
#include <RHI/Resource/Views/ShaderResource.hpp>
#include <RHI/Resource/Views/UnorderedAccess.hpp>
#include <RHI/Resource/Views/RenderTarget.hpp>
#include <RHI/Resource/Views/DepthStencil.hpp>

#include <Log/Logger.hpp>

namespace ranges = std::ranges;
namespace views  = std::views;

namespace Neon::Renderer
{
    Ptr<IMaterial> IMaterial::Create(
        const GenericMaterialBuilder<false>& Builder)
    {
        uint32_t LocalResourceDescriptorSize = 0,
                 LocalSamplerDescriptorSize  = 0;

        Ptr<Material> Mat{ NEON_NEW Material(Builder, LocalResourceDescriptorSize, LocalSamplerDescriptorSize) };
        Mat->CreateDefaultInstance(LocalResourceDescriptorSize, LocalSamplerDescriptorSize);
        return Mat;
    }

    Ptr<IMaterial> IMaterial::Create(
        const GenericMaterialBuilder<true>& Builder)
    {
        uint32_t LocalResourceDescriptorSize = 0,
                 LocalSamplerDescriptorSize  = 0;

        Ptr<Material> Mat{ NEON_NEW Material(Builder, LocalResourceDescriptorSize, LocalSamplerDescriptorSize) };
        Mat->CreateDefaultInstance(LocalResourceDescriptorSize, LocalSamplerDescriptorSize);
        return Mat;
    }

    //

    static void CreateDescriptorIfNeeded(
        RHI::DescriptorHeapHandle& Descriptor,
        RHI::DescriptorType        Type,
        uint32_t                   Count)
    {
        auto Allocator = RHI::IStaticDescriptorHeap::Get(Type);
        if (Count)
        {
            Descriptor = Allocator->Allocate(Count);
        }
    }

    template<bool _Compute>
    void Material_CreateDescriptors(
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

        CreateDescriptorIfNeeded(Mat->m_SharedResourceDescriptor, RHI::DescriptorType::ResourceView, TableSharedResourceCount);
        CreateDescriptorIfNeeded(Mat->m_SharedSamplerDescriptor, RHI::DescriptorType::Sampler, TableSharedSamplerCount);

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
                .Topology      = Builder.Topology(),

                .StripCut = Builder.StripCut(),
                .DSFormat = Builder.DepthStencilFormat()
            };

            {
                auto& InputLayout = Builder.InputLayout();
                if (InputLayout)
                {
                    PipelineDesc.Input = *InputLayout;
                }
                else if (!Builder.NoVertexInput())
                {
                    PipelineDesc.VertexShader->CreateInputLayout(PipelineDesc.Input);
                }
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
        const RenderMaterialBuilder& Builder,
        uint32_t&                    LocalResourceDescriptorSize,
        uint32_t&                    LocalSamplerDescriptorSize)
    {
        Material_CreateDescriptors(
            Builder,
            this,
            LocalResourceDescriptorSize,
            LocalSamplerDescriptorSize);

        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        const ComputeMaterialBuilder& Builder,
        uint32_t&                     LocalResourceDescriptorSize,
        uint32_t&                     LocalSamplerDescriptorSize)
    {
        Material_CreateDescriptors(
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
            auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
            Allocator->Free(m_SharedResourceDescriptor);
        }
        if (m_SharedSamplerDescriptor)
        {
            auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::Sampler);
            Allocator->Free(m_SharedSamplerDescriptor);
        }
    }

    Ptr<IMaterialInstance> Material::CreateInstance()
    {
        return m_DefaultInstance->CreateInstance();
    }

    const Ptr<IMaterialInstance>& Material::GetDefaultInstance()
    {
        return m_DefaultInstance;
    }

    void Material::Bind(
        RHI::IGraphicsCommandList* CommandList)
    {
        CommandList->SetRootSignature(m_RootSignature);
        CommandList->SetPipelineState(m_PipelineState);
    }

    void Material::CreateDefaultInstance(
        uint32_t LocalResourceDescriptorSize,
        uint32_t LocalSamplerDescriptorSize)
    {
        NEON_ASSERT(!m_DefaultInstance);
        m_DefaultInstance.reset(NEON_NEW MaterialInstance(
            shared_from_this(),
            LocalResourceDescriptorSize,
            LocalSamplerDescriptorSize));
    }

    //

    MaterialInstance::MaterialInstance(
        Ptr<IMaterial> Mat,
        uint32_t       LocaResourceDescriptorSize,
        uint32_t       LocaSamplerDescriptorSize)
    {
        m_ParentMaterial = std::move(Mat);

        CreateDescriptorIfNeeded(m_ResourceDescriptor, RHI::DescriptorType::ResourceView, LocaResourceDescriptorSize);
        CreateDescriptorIfNeeded(m_SamplerDescriptor, RHI::DescriptorType::Sampler, LocaSamplerDescriptorSize);
    }

    Ptr<IMaterialInstance> MaterialInstance::CreateInstance()
    {
        return Ptr<IMaterialInstance>(NEON_NEW MaterialInstance(
            GetParentMaterial(),
            m_ResourceDescriptor.Size,
            m_SamplerDescriptor.Size));
    }

    MaterialInstance::~MaterialInstance()
    {
        if (m_ResourceDescriptor)
        {
            auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
            Allocator->Free(m_ResourceDescriptor);
        }
        if (m_SamplerDescriptor)
        {
            auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::Sampler);
            Allocator->Free(m_SamplerDescriptor);
        }
    }

    const Ptr<IMaterial>& IMaterialInstance::GetParentMaterial() const
    {
        return m_ParentMaterial;
    }

    void MaterialInstance::GetDescriptor(
        RHI::DescriptorHeapHandle* OutResourceDescriptor,
        RHI::DescriptorHeapHandle* OutSamplerDescriptor) const
    {
        if (OutResourceDescriptor)
        {
            *OutResourceDescriptor = m_ResourceDescriptor;
        }
        if (OutSamplerDescriptor)
        {
            *OutSamplerDescriptor = m_SamplerDescriptor;
        }
    }

    void MaterialInstance::SetResource(
        const std::string&             Name,
        const Ptr<RHI::IGpuResource>   Resource,
        const RHI::DescriptorViewDesc& Desc,
        size_t                         ArrayIndex)
    {
        auto Mat    = static_cast<Material*>(m_ParentMaterial.get());
        auto Layout = Mat->m_EntryMap.find(Name);
        if (Layout == Mat->m_EntryMap.end())
        {
            NEON_WARNING("Material", "Failed to find resource: {}", Name);
            return;
        }

        if (auto DescriptorEntry = std::get_if<Material::DescriptorEntry>(&Layout->second.Entry))
        {
            DescriptorHeapHandle& Handle = Layout->second.IsShared ? Mat->m_SharedResourceDescriptor : m_ResourceDescriptor;

            NEON_ASSERT(ArrayIndex < DescriptorEntry->Descs.size());

            auto& EntryDesc     = DescriptorEntry->Descs[ArrayIndex];
            auto& EntryResource = DescriptorEntry->Resources[ArrayIndex];

            EntryDesc     = Desc;
            EntryResource = Resource;

            std::visit(
                VariantVisitor{
                    [](const std::monostate&)
                    {
                        NEON_ASSERT(false, "Invalid view type");
                    },
                    [&Handle, this](const RHI::CBVDesc& Desc)
                    {
                        RHI::Views::ConstantBuffer View{ Handle };
                        View.Bind(Desc);
                    },
                    [&Handle, Resource, this](const std::optional<RHI::SRVDesc>& Desc)
                    {
                        RHI::Views::ShaderResource View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                    },
                    [&Handle, Resource, this](const std::optional<RHI::UAVDesc>& Desc)
                    {
                        RHI::Views::UnorderedAccess View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                    },
                    [&Handle, Resource, this](const std::optional<RHI::RTVDesc>& Desc)
                    {
                        RHI::Views::RenderTarget View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                    },
                    [&Handle, Resource, this](const std::optional<RHI::DSVDesc>& Desc)
                    {
                        RHI::Views::DepthStencil View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr);
                    } },
                EntryDesc);
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a resource", Name);
        }
    }
} // namespace Neon::Renderer