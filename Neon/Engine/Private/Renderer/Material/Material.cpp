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
        RHI::DescriptorHeapHandle&     Descriptor,
        RHI::IDescriptorHeapAllocator* Allocator,
        uint32_t                       Count)
    {
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

        auto& VarMap = Builder.VarMap();
        VarMap.ForEachVariable(
            [&RootSigBuilder,
             &TableResourceCount,
             &TableSharedResourceCount,
             &TableSamplerCount,
             &TableSharedSamplerCount](
                const MaterialVariableMap::View& View)
            {
                switch (View.Type())
                {
                case MaterialVarType::Buffer:
                case MaterialVarType::Resource:
                case MaterialVarType::RWResource:
                {
                    if (View.Flags().Test(EMaterialVarFlags::Shared))
                    {
                        TableResourceCount += View.ArraySize();
                    }
                    else
                    {
                        TableSharedResourceCount += View.ArraySize();
                    }
                    break;
                }
                default:
                    NEON_ASSERT(false);
                }
            });

        auto ResourceDescriptor = Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::ResourceView, false);
        auto SamplerDescriptor  = Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::Sampler, false);

        CreateDescriptorIfNeeded(Mat->m_SharedResourceDescriptor, ResourceDescriptor, TableSharedResourceCount);
        CreateDescriptorIfNeeded(Mat->m_SharedSamplerDescriptor, SamplerDescriptor, TableSharedSamplerCount);

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
        CreateDescriptorIfNeeded(m_ResourceDescriptor, m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::ResourceView, true), LocaResourceDescriptorSize);
        CreateDescriptorIfNeeded(m_SamplerDescriptor, m_Swapchain->GetDescriptorHeapManager(RHI::DescriptorType::Sampler, true), LocaSamplerDescriptorSize);
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