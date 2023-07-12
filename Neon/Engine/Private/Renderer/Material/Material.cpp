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
        return Ptr<Material>{ NEON_NEW Material(Builder) };
    }

    Ptr<IMaterial> IMaterial::Create(
        const GenericMaterialBuilder<true>& Builder)
    {
        return Ptr<Material>{ NEON_NEW Material(Builder) };
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
        Material*                               Mat)
    {
        RHI::RootSignatureBuilder RootSigBuilder;

        uint32_t TableResourceCount       = 0,
                 TableSharedResourceCount = 0;
        uint32_t TableSamplerCount        = 0,
                 TableSharedSamplerCount  = 0;

        struct BatchedDescriptorEntry
        {
            Material::LayoutEntry* LayoutEntry;
            ShaderBinding          Binding;
            uint32_t               Size;

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
                    size_t BatchIndex  = View.Type() == MaterialVarType::Sampler ? 1 : 0;
                    auto&  Descriptors = BatchedDescriptorEntries[BatchIndex][View.Visibility()][View.Type()];

                    Material::DescriptorEntry DescriptorEntry{};

                    DescriptorEntry.Descs.resize(View.ArraySize());
                    DescriptorEntry.Resources.resize(View.ArraySize());

                    const bool IsShared        = View.Flags().Test(EMaterialVarFlags::Shared);
                    const bool IsSharedUnified = View.Flags().Test(EMaterialVarFlags::SharedUnified);
                    const bool IsSharedAny     = IsShared || IsSharedUnified;

                    // Shared resources are allocated in a limited descriptor space
                    // while unified resources are allocated in a per-material descriptor space
                    const uint32_t EntrySize      = IsShared ? View.ArraySize() : Material::UnboundedTableSize;
                    const uint32_t DescriptorSize = IsSharedUnified ? View.ArraySize() : Material::UnboundedTableSize;

                    auto& DescriptorCount =
                        View.Type() == MaterialVarType::Sampler
                            ? (IsSharedAny ? TableSharedSamplerCount : TableSamplerCount)
                            : (IsSharedAny ? TableSharedResourceCount : TableResourceCount);

                    DescriptorEntry.Offset = DescriptorCount;
                    DescriptorCount += DescriptorSize;

                    auto& LayoutEntry    = (*Mat->m_EntryMap)[View.Name()];
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

        Mat->m_SharedDescriptors = std::make_shared<Material::UnqiueDescriptorHeapHandle>(TableSharedResourceCount, TableSharedSamplerCount);
        Mat->m_LocalDescriptors  = Material::UnqiueDescriptorHeapHandle(TableResourceCount, TableSamplerCount);
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
                    NEON_WARNING_TAG("Material", "Failed to load shader module: {:X}", ModuleHandle.ModuleId);
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
        const RenderMaterialBuilder& Builder) :
        m_EntryMap(std::make_shared<LayoutEntryMap>())
    {
        Material_CreateDescriptors(
            Builder,
            this);

        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        const ComputeMaterialBuilder& Builder) :
        m_EntryMap(std::make_shared<LayoutEntryMap>())
    {
        Material_CreateDescriptors(
            Builder,
            this);

        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        Material* Other) :
        m_SharedDescriptors(Other->m_SharedDescriptors),
        m_LocalDescriptors(Other->m_LocalDescriptors.ResourceDescriptors.Size, Other->m_LocalDescriptors.SamplerDescriptors.Size),
        m_EntryMap(Other->m_EntryMap)
    {
        m_RootSignature = Other->m_RootSignature;
        m_PipelineState = Other->m_PipelineState;
    }

    Ptr<IMaterial> Material::CreateInstance()
    {
        return Ptr<IMaterial>(NEON_NEW Material(this));
    }

    //

    void Material::GetDescriptor(
        bool                       Local,
        RHI::DescriptorHeapHandle* OutResourceDescriptor,
        RHI::DescriptorHeapHandle* OutSamplerDescriptor) const
    {
        if (OutResourceDescriptor)
        {
            *OutResourceDescriptor = Local ? m_LocalDescriptors.ResourceDescriptors : m_SharedDescriptors->ResourceDescriptors;
        }
        if (OutSamplerDescriptor)
        {
            *OutSamplerDescriptor = Local ? m_LocalDescriptors.SamplerDescriptors : m_SharedDescriptors->SamplerDescriptors;
        }
    }

    const Ptr<RHI::IRootSignature>& IMaterial::GetRootSignature() const noexcept
    {
        return m_RootSignature;
    }

    const Ptr<RHI::IPipelineState>& IMaterial::GetPipelineState() const noexcept
    {
        return m_PipelineState;
    }

    void Material::ApplyAll(
        RHI::IGraphicsCommandList*       CommandList,
        const RHI::DescriptorHeapHandle& ResourceDescriptor,
        const RHI::DescriptorHeapHandle& SamplerDescriptor) const
    {
        uint32_t SizeOfSharedResource = m_SharedDescriptors->ResourceDescriptors.Size;
        uint32_t SizeOfSharedSampler  = m_SharedDescriptors->SamplerDescriptors.Size;

        for (auto& [Entry, RootIndex, IsShared] : *m_EntryMap | views::values)
        {
            std::visit(
                VariantVisitor{
                    [&](const Material::DescriptorEntry& Descriptor)
                    {
                        uint32_t Offset = IsShared ? 0 : SizeOfSharedResource;
                        CommandList->SetDescriptorTable(RootIndex, ResourceDescriptor.GetGpuHandle(Descriptor.Offset + Offset));
                    },
                    [&](const Material::SamplerEntry& Sampler)
                    {
                        uint32_t Offset = IsShared ? 0 : SizeOfSharedSampler;
                        CommandList->SetDescriptorTable(RootIndex, SamplerDescriptor.GetGpuHandle(Sampler.Offset + Offset));
                    },
                    [&](const Material::ConstantEntry& Constant)
                    {
                        CommandList->SetConstants(RootIndex, Constant.Data.data(), Constant.Data.size() / sizeof(uint32_t));
                    } },
                Entry);
        }
    }

    //

    void Material::SetResource(
        const std::string&             Name,
        const Ptr<RHI::IGpuResource>&  Resource,
        const RHI::DescriptorViewDesc& Desc,
        uint32_t                       ArrayIndex,
        const Ptr<RHI::IGpuResource>&  UavCounter)
    {
        auto Layout = m_EntryMap->find(Name);
        if (Layout == m_EntryMap->end())
        {
            NEON_WARNING_TAG("Material", "Failed to find resource: {}", Name);
            return;
        }

        if (auto DescriptorEntry = std::get_if<Material::DescriptorEntry>(&Layout->second.Entry))
        {
            auto& Handle = Layout->second.IsShared ? m_SharedDescriptors->ResourceDescriptors : m_LocalDescriptors.ResourceDescriptors;

            NEON_ASSERT(ArrayIndex < DescriptorEntry->Descs.size());

            auto& EntryDesc     = DescriptorEntry->Descs[ArrayIndex];
            auto& EntryResource = DescriptorEntry->Resources[ArrayIndex];

            EntryDesc     = Desc;
            EntryResource = Resource;

            uint32_t DescriptorOffset = ArrayIndex + DescriptorEntry->Offset;
            std::visit(
                VariantVisitor{
                    [](const std::monostate&)
                    {
                        NEON_ASSERT(false, "Invalid view type");
                    },
                    [&Handle, this, DescriptorOffset](
                        const RHI::CBVDesc& Desc)
                    {
                        RHI::Views::ConstantBuffer View{ Handle };
                        View.Bind(Desc, DescriptorOffset);
                    },
                    [&Handle, &Resource, this, DescriptorOffset](
                        const std::optional<RHI::SRVDesc>& Desc)
                    {
                        RHI::Views::ShaderResource View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                    },
                    [&Handle, &Resource, &UavCounter, this, DescriptorOffset](
                        const std::optional<RHI::UAVDesc>& Desc)
                    {
                        RHI::Views::UnorderedAccess View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, UavCounter.get(), DescriptorOffset);
                    },
                    [&Handle, &Resource, this, DescriptorOffset](
                        const std::optional<RHI::RTVDesc>& Desc)
                    {
                        RHI::Views::RenderTarget View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                    },
                    [&Handle, &Resource, this, DescriptorOffset](
                        const std::optional<RHI::DSVDesc>& Desc)
                    {
                        RHI::Views::DepthStencil View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                    } },
                EntryDesc);
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a resource", Name);
        }
    }

    void Material::SetSampler(
        const std::string&      Name,
        const RHI::SamplerDesc& Desc,
        uint32_t                ArrayIndex)
    {
    }

    //

    Material::UnqiueDescriptorHeapHandle::UnqiueDescriptorHeapHandle(
        uint32_t ResourceDescriptorSize,
        uint32_t SamplerDescriptorSize)
    {
        CreateDescriptorIfNeeded(ResourceDescriptors, RHI::DescriptorType::ResourceView, ResourceDescriptorSize);
        CreateDescriptorIfNeeded(SamplerDescriptors, RHI::DescriptorType::Sampler, SamplerDescriptorSize);
    }

    Material::UnqiueDescriptorHeapHandle::UnqiueDescriptorHeapHandle(UnqiueDescriptorHeapHandle&&)                    = default;
    auto Material::UnqiueDescriptorHeapHandle::operator=(UnqiueDescriptorHeapHandle&&) -> UnqiueDescriptorHeapHandle& = default;

    Material::UnqiueDescriptorHeapHandle::~UnqiueDescriptorHeapHandle()
    {
        if (ResourceDescriptors)
        {
            auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
            Allocator->Free(ResourceDescriptors);
        }
        if (SamplerDescriptors)
        {
            auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::Sampler);
            Allocator->Free(SamplerDescriptors);
        }
    }

    //

    MaterialBinder::MaterialBinder(std::span<IMaterial*> Materials) :
        m_Materials(Materials)
    {
#if NEON_DEBUG
        NEON_ASSERT(!m_Materials.empty(), "Materials are empty");
        auto FirstMaterial = m_Materials[0];
        for (auto Mat : m_Materials)
        {
            NEON_ASSERT(Mat, "Material is null");
            NEON_ASSERT(FirstMaterial->GetPipelineState() == Mat->GetPipelineState(), "Materials have different pipeline states");
        }
#endif
    }

    void MaterialBinder::Bind(
        RHI::IGraphicsCommandList* CommandList)
    {
        CommandList->SetRootSignature(m_Materials[0]->GetRootSignature());
        CommandList->SetPipelineState(m_Materials[0]->GetPipelineState());
    }

    void MaterialBinder::BindParams(
        RHI::IGraphicsCommandList* CommandList)
    {
        uint32_t GPUResourceDescriptorSize = 0,
                 GPUSamplerDescriptorSize  = 0;

        std::vector<RHI::IDescriptorHeap::CopyInfo> ResourceDescriptors, SamplerDescriptors;

        RHI::DescriptorHeapHandle ResourceDescriptor, SamplerDescriptor;

        //

        auto SaveDescriptor = [&](bool IsSampler)
        {
            auto& Descriptor  = IsSampler ? SamplerDescriptor : ResourceDescriptor;
            auto& Descriptors = IsSampler ? SamplerDescriptors : ResourceDescriptors;
            auto& TotalSize   = IsSampler ? GPUSamplerDescriptorSize : GPUResourceDescriptorSize;

            if (Descriptor.Size)
            {
                Descriptors.emplace_back(Descriptor.GetCpuHandle(), Descriptor.Size);
                TotalSize += Descriptor.Size;
            }
        };

        auto SaveDescriptors =
            [&]()
        {
            SaveDescriptor(false);
            SaveDescriptor(true);
        };

        //

        auto UploadDescriptor =
            [&](bool IsSampler)
        {
            RHI::DescriptorType Type;
            uint32_t            DescriptorSize;

            if (IsSampler)
            {
                Type           = RHI::DescriptorType::Sampler;
                DescriptorSize = GPUSamplerDescriptorSize;
            }
            else
            {
                Type           = RHI::DescriptorType::ResourceView;
                DescriptorSize = GPUResourceDescriptorSize;
            }

            if (!DescriptorSize)
            {
                return;
            }

            auto& Descriptors = IsSampler ? SamplerDescriptors : ResourceDescriptors;
            auto& Descriptor  = IsSampler ? SamplerDescriptor : ResourceDescriptor;

            auto ResourceTable = RHI::IFrameDescriptorHeap::Get(Type);
            Descriptor         = ResourceTable->Allocate(DescriptorSize);

            RHI::IDescriptorHeap::CopyInfo Destination{
                .Descriptor = Descriptor.GetCpuHandle(),
                .CopySize   = Descriptor.Size
            };
            RHI::IDescriptorHeap::Copy(Type, Descriptors, { &Destination, 1 });
        };

        auto UploadDescriptors =
            [&]()
        {
            UploadDescriptor(false);
            UploadDescriptor(true);
        };

        //

        // Save shared descriptors
        m_Materials[0]->GetDescriptor(false, &ResourceDescriptor, &SamplerDescriptor);
        SaveDescriptors();

        // Save local descriptors
        for (auto Instance : m_Materials)
        {
            Instance->GetDescriptor(true, &ResourceDescriptor, &SamplerDescriptor);
            SaveDescriptors();
        }

        // Allocate frame descriptors and upload to it
        UploadDescriptors();
        m_Materials[0]->ApplyAll(CommandList, ResourceDescriptor, SamplerDescriptor);
    }

    void MaterialBinder::BindAll(
        RHI::IGraphicsCommandList* CommandList)
    {
        Bind(CommandList);
        BindParams(CommandList);
    }

    //

    void MaterialTable::Reset()
    {
        m_Materials.clear();
    }

    int MaterialTable::Append(
        IMaterial* Material)
    {
        for (int i = 0; i < m_Materials.size(); i++)
        {
            if (m_Materials[i] == Material)
            {
                return i;
            }
        }
        m_Materials.push_back(Material);
        return int(m_Materials.size() - 1);
    }
} // namespace Neon::Renderer