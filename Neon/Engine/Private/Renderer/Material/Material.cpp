#include <EnginePCH.hpp>
#include <Renderer/Material/Builder.hpp>
#include <Renderer/Material/Material.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>

#include <Log/Logger.hpp>

namespace Neon::Renderer
{
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

        Mat->m_RootSignature = RHI::IRootSignature::Create(RootSigBuilder);

        //

        CreateDescriptorIfNeeded(Mat->m_SharedResourceDescriptor, Swapchain, RHI::DescriptorType::ResourceView, TableSharedResourceCount);
        CreateDescriptorIfNeeded(Mat->m_SharedSamplerDescriptor, Swapchain, RHI::DescriptorType::Sampler, TableSharedSamplerCount);

        LocaResourceDescriptorSize = TableResourceCount;
        LocaSamplerDescriptorSize  = TableSamplerCount;
    }

    //

    Material::Material(
        RHI::ISwapchain*             Swapchain,
        const RenderMaterialBuilder& Builder) :
        m_Swapchain(Swapchain)
    {
        uint32_t LocalResourceDescriptorSize = 0,
                 LocalSamplerDescriptorSize  = 0;

        Material_CreateDescriptors(
            Swapchain,
            Builder,
            this,
            LocalResourceDescriptorSize,
            LocalSamplerDescriptorSize);

        m_DefaultInstace = Ptr<MaterialInstance>(
            NEON_NEW MaterialInstance{
                shared_from_this(),
                LocalResourceDescriptorSize,
                LocalSamplerDescriptorSize });
    }

    Material::Material(
        RHI::ISwapchain*              Swapchain,
        const ComputeMaterialBuilder& Builder)
    {
        uint32_t LocalResourceDescriptorSize = 0,
                 LocalSamplerDescriptorSize  = 0;

        Material_CreateDescriptors(
            Swapchain,
            Builder,
            this,
            LocalResourceDescriptorSize,
            LocalSamplerDescriptorSize);

        m_DefaultInstace = Ptr<MaterialInstance>(
            NEON_NEW MaterialInstance{
                shared_from_this(),
                LocalResourceDescriptorSize,
                LocalSamplerDescriptorSize });
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