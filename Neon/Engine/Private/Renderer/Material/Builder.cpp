#include <EnginePCH.hpp>
#include <Renderer/Material/Builder.hpp>
#include <RHI/Swapchain.hpp>

namespace Neon::Renderer
{
    MaterialMetaData::MaterialMetaData(
        const MaterialMetaDataBuilder& Builder,
        RHI::ISwapchain*               SwapChain) :
        m_SwapChain(SwapChain)
    {
        RHI::RootSignatureBuilder RootSigBuilder;
        RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout);

        uint32_t RootIndex               = 0;
        uint32_t ResourceDescriptorIndex = 0, SamplerDescriptorIndex = 0;

        for (auto& Entry : Builder.m_Entries)
        {
            auto& ResourceData     = m_LayoutMap[Entry.m_Name];
            ResourceData.RootIndex = RootIndex;

            auto [Register, Space] = Entry.m_Binding;

            if (Entry.m_PreferDescriptor)
            {
                RHI::RootDescriptorTable Table;

                switch (Entry.m_Type)
                {
                case MaterialDataType::ConstantBuffer:
                    Table.AddCbvRange(Register, Space, Entry.m_ArraySize);
                    break;
                case MaterialDataType::Resource:
                    Table.AddSrvRange(Register, Space, Entry.m_ArraySize);
                    break;
                case MaterialDataType::UavResource:
                    Table.AddUavRange(Register, Space, Entry.m_ArraySize);
                    break;
                case MaterialDataType::Sampler:
                    Table.AddSamplerRange(Register, Space, Entry.m_ArraySize);
                    break;
                default:
                    std::unreachable();
                }

                RootSigBuilder.AddDescriptorTable(Table, Entry.m_Visiblity);

                if (Entry.m_Type != MaterialDataType::Sampler)
                {
                    auto& Descriptor = ResourceData.Entry.emplace<DescriptorEntry>() = {
                        .Descs  = std::get<std::vector<RHI::DescriptorViewDesc>>(Entry.m_DescriptorDesc),
                        .Offset = ResourceDescriptorIndex,
                        .Type   = Entry.m_Type,
                    };
                    Descriptor.Resources.resize(Descriptor.Descs.size());
                    ResourceDescriptorIndex += uint32_t(Descriptor.Descs.size());
                }
                else
                {
                    auto& Descriptor = ResourceData.Entry.emplace<SamplerEntry>() = {
                        .Descs = std::get<std::vector<RHI::SamplerDesc>>(Entry.m_DescriptorDesc),
                        .Count = Entry.m_ArraySize,
                    };
                    SamplerDescriptorIndex += uint32_t(Descriptor.Descs.size());
                }

                RootIndex++;
            }
            else
            {
                switch (Entry.m_Type)
                {
                case MaterialDataType::Constants:
                {
                    RootIndex++;
                    size_t DataSize = Entry.m_DataSize * Entry.m_ArraySize;
                    ResourceData.Entry.emplace<ConstantEntry>(std::vector<uint8_t>(DataSize), Entry.m_DataSize);
                    RootSigBuilder.Add32BitConstants(Register, Space, uint32_t(DataSize / sizeof(int)), Entry.m_Visiblity);
                    break;
                }
                case MaterialDataType::ConstantBuffer:
                {
                    auto& Root    = ResourceData.Entry.emplace<RootEntry>();
                    Root.ViewType = MaterialDataType::ConstantBuffer;
                    Root.Resources.resize(Entry.m_ArraySize);
                    for (uint16_t i = 0; i < Entry.m_ArraySize; i++)
                    {
                        RootIndex++;
                        RootSigBuilder.AddConstantBufferView(Register + i, Space, Entry.m_Visiblity);
                    }
                    break;
                }
                case MaterialDataType::Resource:
                {
                    auto& Root    = ResourceData.Entry.emplace<RootEntry>();
                    Root.ViewType = MaterialDataType::Resource;
                    Root.Resources.resize(Entry.m_ArraySize);
                    for (uint16_t i = 0; i < Entry.m_ArraySize; i++)
                    {
                        RootIndex++;
                        RootSigBuilder.AddShaderResourceView(Register + i, Space, Entry.m_Visiblity);
                    }
                    break;
                }
                case MaterialDataType::UavResource:
                {
                    auto& Root    = ResourceData.Entry.emplace<RootEntry>();
                    Root.ViewType = MaterialDataType::UavResource;
                    Root.Resources.resize(Entry.m_ArraySize);
                    for (uint16_t i = 0; i < Entry.m_ArraySize; i++)
                    {
                        RootIndex++;
                        RootSigBuilder.AddUnorderedAccessView(Register + i, Space, Entry.m_Visiblity);
                    }
                    break;
                }
                case MaterialDataType::Sampler:
                {
                    auto&    SamplerDesc = std::get<std::vector<RHI::SamplerDesc>>(Entry.m_DescriptorDesc);
                    uint32_t RegOffset   = Register;
                    for (auto& Sampler : SamplerDesc)
                    {
                        RHI::StaticSamplerDesc StaticSampler = { Sampler };
                        StaticSampler.ShaderRegister         = RegOffset++;
                        StaticSampler.RegisterSpace          = Space;
                        StaticSampler.Visibility             = Entry.m_Visiblity;
                        RootSigBuilder.AddSampler(std::move(StaticSampler));
                    }
                    break;
                }
                default:
                    std::unreachable();
                }
            }

            for (auto& [Name, Index] : Entry.m_References)
            {
                m_IndirectResources[Name] = { Entry.m_Name, Index };
            }
        }

        m_RootSignature = RHI::IRootSignature::Create(RootSigBuilder);
        AllocateDescriptors(ResourceDescriptorIndex, SamplerDescriptorIndex);
    }

    MaterialMetaData::MaterialMetaData(
        const MaterialMetaData& MetaData) :
        m_RootSignature(MetaData.m_RootSignature),
        m_IndirectResources(MetaData.m_IndirectResources),
        m_LayoutMap(MetaData.m_LayoutMap)
    {
        AllocateDescriptors(
            MetaData.m_ResourceDescriptor.GetHandle().Size,
            MetaData.m_SamplerDescriptor.GetHandle().Size);
    }

    MaterialMetaData& MaterialMetaData::operator=(
        const MaterialMetaData& MetaData)
    {
        if (this != &MetaData)
        {
            m_RootSignature     = MetaData.m_RootSignature;
            m_IndirectResources = MetaData.m_IndirectResources;
            m_LayoutMap         = MetaData.m_LayoutMap;
            AllocateDescriptors(
                MetaData.m_ResourceDescriptor.GetHandle().Size,
                MetaData.m_SamplerDescriptor.GetHandle().Size);
        }
        return *this;
    }

    //

    MaterialMetaData::MaterialMetaData(
        MaterialMetaData&& MetaData) noexcept :
        m_RootSignature(std::move(MetaData.m_RootSignature)),
        m_IndirectResources(std::move(MetaData.m_IndirectResources)),
        m_LayoutMap(std::move(MetaData.m_LayoutMap)),
        m_ResourceDescriptor(std::exchange(MetaData.m_ResourceDescriptor, {})),
        m_SamplerDescriptor(std::exchange(MetaData.m_SamplerDescriptor, {}))
    {
    }

    MaterialMetaData& MaterialMetaData::operator=(
        MaterialMetaData&& MetaData) noexcept
    {
        if (this != &MetaData)
        {
            try
            {
                DestroyDescriptors();
            }
            catch (...)
            {
            }

            m_RootSignature      = std::move(MetaData.m_RootSignature);
            m_IndirectResources  = std::move(MetaData.m_IndirectResources);
            m_LayoutMap          = std::move(MetaData.m_LayoutMap);
            m_ResourceDescriptor = std::exchange(MetaData.m_ResourceDescriptor, {});
            m_SamplerDescriptor  = std::exchange(MetaData.m_SamplerDescriptor, {});
        }
        return *this;
    }

    void MaterialMetaData::AllocateDescriptors(
        uint32_t ResourceCount,
        uint32_t SamplerCount)
    {
        if (ResourceCount)
        {
            m_ResourceDescriptor = RHI::Views::Generic(
                ResourceCount,
                m_SwapChain->GetDescriptorHeapManager(RHI::DescriptorType::ResourceView, false));
        }
        if (SamplerCount)
        {
            m_SamplerDescriptor = RHI::Views::Generic(
                ResourceCount,
                m_SwapChain->GetDescriptorHeapManager(RHI::DescriptorType::Sampler, false));
        }
    }

    //

    void MaterialMetaData::Update(
        RHI::ICommonCommandList* Context)
    {
        auto& ResourceHeap = Context.GetResourceHeap();
        auto& SamplerHeap  = Context.GetSamplerHeap();

        for (auto& [Entry, RootIndex] : m_LayoutMap | std::views::values)
        {
            switch (EntryType(Entry.index()))
            {
            case EntryType::Constant:
            {
                auto& Constant = std::get<ConstantEntry>(Entry);
                Context.SetConstants(RootIndex, Constant.Data.data(), Constant.Data.size() / sizeof(int));
                break;
            }

            case EntryType::Root:
            {
                auto& [Resource, ViewType] = std::get<RootEntry>(Entry);
                for (uint32_t i = 0; i < uint32_t(Resource.size()); i++)
                {
                    Context.SetResourceView(ViewType, RootIndex + i, Resource[i]);
                }
                break;
            }

            case EntryType::Descriptor:
            {
                auto& Descriptor = std::get<DescriptorEntry>(Entry);

                auto SrcDesc = m_ResourceDescriptor.Heap->GetCPUAddress(
                    m_ResourceDescriptor.Offset + Descriptor.Offset);
                auto DstDesc = ResourceHeap.Heap->GetGPUAddress(
                    ResourceHeap.Offset + Descriptor.Offset);

                ResourceHeap.Heap->Copy(
                    ResourceHeap.Offset + Descriptor.Offset,
                    { .Descriptor = SrcDesc,
                      .CopySize   = Descriptor.Descs.size() });

                Context.SetDescriptorTable(
                    RootIndex,
                    DstDesc);
                break;
            }

            case EntryType::Sampler:
            {
                auto& Sampler = std::get<SamplerEntry>(Entry);

                auto SrcDesc = m_SamplerDescriptor.Heap->GetCPUAddress(
                    m_SamplerDescriptor.Offset + Sampler.Offset);
                auto DstDesc = SamplerHeap.Heap->GetGPUAddress(
                    SamplerHeap.Offset + Sampler.Offset);

                SamplerHeap.Heap->Copy(
                    SamplerHeap.Offset + Sampler.Offset,
                    { .Descriptor = SrcDesc,
                      .CopySize   = Sampler.Descs.size() });

                Context.SetDescriptorTable(
                    RootIndex,
                    DstDesc);
                break;
            }
            }
        }
    }

    //

    const RHI::RootSignature& MaterialMetaData::GetRootSignature() const
    {
        return m_RootSignature;
    }

    void MaterialMetaData::SetConstants(
        const StringU8& Name,
        size_t          ArrayIndex,
        size_t          Offset,
        const void*     Data,
        size_t          DataSize)
    {
        auto FinalName = &Name;
        if (auto It = m_IndirectResources.find(Name); It != m_IndirectResources.end())
        {
            FinalName = &It->second.first;
            ArrayIndex += It->second.second;
        }
        SetConstants_Internal(*FinalName, ArrayIndex, Offset, Data, DataSize);
    }

    //

    void MaterialMetaData::SetResource(
        const StringU8&                               Name,
        size_t                                        ArrayIndex,
        const GraphicsResourcePtr&                    Resource,
        const std::optional<RHI::DescriptorViewDesc>& Desc)
    {
        auto FinalName = &Name;
        if (auto It = m_IndirectResources.find(Name); It != m_IndirectResources.end())
        {
            FinalName = &It->second.first;
            ArrayIndex += It->second.second;
        }
        SetResource_Internal(*FinalName, ArrayIndex, Resource, Desc);
    }

    //

    void MaterialMetaData::GetConstants(
        const StringU8& Name,
        size_t          ArrayIndex,
        size_t          Offset,
        void*           Data,
        size_t          DataSize) const
    {
        auto FinalName = &Name;
        if (auto It = m_IndirectResources.find(Name); It != m_IndirectResources.end())
        {
            FinalName = &It->second.first;
            ArrayIndex += It->second.second;
        }
        GetConstants_Internal(*FinalName, ArrayIndex, Offset, Data, DataSize);
    }

    void MaterialMetaData::GetResource(
        const StringU8&          Name,
        size_t                   ArrayIndex,
        GraphicsResourcePtr*     Resource,
        RHI::DescriptorViewDesc* Desc) const
    {
        auto FinalName = &Name;
        if (auto It = m_IndirectResources.find(Name); It != m_IndirectResources.end())
        {
            FinalName = &It->second.first;
            ArrayIndex += It->second.second;
        }
        GetResource_Internal(*FinalName, ArrayIndex, Resource, Desc);
    }

    //

    void MaterialMetaData::SetConstants_Internal(
        const StringU8& Name,
        size_t          ArrayIndex,
        size_t          ByteOffset,
        const void*     Data,
        size_t          DataSize)
    {
        auto It = m_LayoutMap.find(Name);
        NEON_ASSERT_MSG(It != m_LayoutMap.end(), "Entry in Layout map doesn't exists");

        auto Entry = &It->second.Entry;
        if (auto Root = std::get_if<RootEntry>(Entry))
        {
            auto Buffer = dynamic_cast<RHI::GraphicsUploadBuffer*>(Root->Resources[ArrayIndex].get());
            NEON_ASSERT(Buffer);
            Buffer->Write(ByteOffset, Data, DataSize);
        }
        else if (auto Constant = std::get_if<ConstantEntry>(Entry))
        {
            std::copy(
                static_cast<const uint8_t*>(Data),
                static_cast<const uint8_t*>(Data) + DataSize,
                Constant->Data.data() + Constant->DataStride * ArrayIndex + ByteOffset);
        }
        else
        {
            std::unreachable();
        }
    }

    //

    void MaterialMetaData::SetResource_Internal(
        const StringU8&                               Name,
        size_t                                        ArrayIndex,
        const GraphicsResourcePtr&                    Resource,
        const std::optional<RHI::DescriptorViewDesc>& Desc)
    {
        auto It = m_LayoutMap.find(Name);
        NEON_ASSERT_MSG(It != m_LayoutMap.end(), "Entry in Layout map doesn't exists");

        auto Entry = &It->second.Entry;
        if (auto Root = std::get_if<RootEntry>(Entry))
        {
            Root->Resources[ArrayIndex] = std::dynamic_pointer_cast<RHI::GraphicsBuffer>(Resource);
        }
        else if (auto Descriptor = std::get_if<DescriptorEntry>(Entry))
        {
            Descriptor->Resources[ArrayIndex] = Resource;
            if (Desc)
            {
                Descriptor->Descs[ArrayIndex] = *Desc;
            }

            switch (Descriptor->Type)
            {
            case MaterialDataType::ConstantBuffer:
            {
                using DescType = RHI::ConstantBufferViewDesc;
                auto& HeapDesc = std::get<DescType>(Descriptor->Descs[ArrayIndex]);

                m_ResourceDescriptor.Heap->CreateConstantBufferView(
                    m_ResourceDescriptor.Offset + Descriptor->Offset + ArrayIndex,
                    dynamic_cast<RHI::GraphicsBuffer*>(Resource.get())->GetGpuAddress(),
                    HeapDesc.ViewSize);
                break;
            }
            case MaterialDataType::Resource:
            {
                using DescType = RHI::ShaderResourceViewDesc;
                auto& HeapDesc = std::get<DescType>(Descriptor->Descs[ArrayIndex]).HeapDesc;

                m_ResourceDescriptor.Heap->CreateShaderResourceView(
                    m_ResourceDescriptor.Offset + Descriptor->Offset + ArrayIndex,
                    Resource ? Resource->Get() : nullptr,
                    HeapDesc.has_value() ? &*HeapDesc : nullptr);
                break;
            }
            case MaterialDataType::UavResource:
            {
                using DescType = RHI::UnorderedAccessViewDesc;
                auto& HeapDesc = std::get<DescType>(Descriptor->Descs[ArrayIndex]).HeapDesc;

                m_ResourceDescriptor.Heap->CreateUnorderedAccessView(
                    m_ResourceDescriptor.Offset + Descriptor->Offset + ArrayIndex,
                    Resource ? Resource->Get() : nullptr,
                    HeapDesc.has_value() ? &*HeapDesc : nullptr,
                    nullptr);
                break;
            }
            default:
                std::unreachable();
            }
        }
        else
        {
            std::unreachable();
        }
    }

    //

    void MaterialMetaData::GetConstants_Internal(
        const StringU8& Name,
        size_t          ArrayIndex,
        size_t          ByteOffset,
        void*           Data,
        size_t          DataSize) const
    {
        auto It = m_LayoutMap.find(Name);
        NEON_ASSERT_MSG(It != m_LayoutMap.end(), "Entry in Layout map doesn't exists");

        auto Entry = &It->second.Entry;
        if (auto Constant = std::get_if<ConstantEntry>(Entry))
        {
            const uint8_t* FromData = Constant->Data.data() + Constant->DataStride * ArrayIndex + ByteOffset;
            std::copy(
                FromData,
                FromData + DataSize,
                static_cast<uint8_t*>(Data));
        }
        else
        {
            std::unreachable();
        }
    }

    void MaterialMetaData::GetResource_Internal(
        const StringU8&          Name,
        size_t                   ArrayIndex,
        GraphicsResourcePtr*     Resource,
        RHI::DescriptorViewDesc* Desc) const
    {
        auto It = m_LayoutMap.find(Name);
        NEON_ASSERT_MSG(It != m_LayoutMap.end(), "Entry in Layout map doesn't exists");

        auto Entry = &It->second.Entry;
        if (auto Root = std::get_if<RootEntry>(Entry))
        {
            *Resource = Root->Resources[ArrayIndex];
        }
        else if (auto Descriptor = std::get_if<DescriptorEntry>(Entry))
        {
            if (Resource)
                *Resource = Descriptor->Resources[ArrayIndex];
            if (Desc)
                *Desc = Descriptor->Descs[ArrayIndex];
        }
        else
        {
            std::unreachable();
        }
    }

    //

    auto MaterialMetaDataBuilder::Entry::SetShader(
        ShaderVisibility Visibility)
        -> Entry&
    {
        switch (Visibility)
        {
        case ShaderVisibility::All:
            m_Visiblity = D3D12_SHADER_VISIBILITY_ALL;
            break;

        case ShaderVisibility::Vertex:
            m_Visiblity = D3D12_SHADER_VISIBILITY_VERTEX;
            break;

        case ShaderVisibility::Domain:
            m_Visiblity = D3D12_SHADER_VISIBILITY_DOMAIN;
            break;

        case ShaderVisibility::Hull:
            m_Visiblity = D3D12_SHADER_VISIBILITY_HULL;
            break;

        case ShaderVisibility::Geometry:
            m_Visiblity = D3D12_SHADER_VISIBILITY_GEOMETRY;
            break;

        case ShaderVisibility::Pixel:
            m_Visiblity = D3D12_SHADER_VISIBILITY_PIXEL;
            break;
        default:
            std::unreachable();
        }
        return *this;
    }

    auto MaterialMetaDataBuilder::Entry::SetBinding(
        uint16_t Register,
        uint16_t Space) -> Entry&
    {
        m_Binding = { Register, Space };
        return *this;
    }

    auto MaterialMetaDataBuilder::Entry::SetReference(
        const StringU8& Name,
        uint32_t        ArrayIndex) -> Entry&
    {
        m_References.emplace_back(Name, ArrayIndex);
        return *this;
    }

    auto MaterialMetaDataBuilder::Entry::SetDescriptor(
        size_t                  ArrayIndex,
        const RHI::SamplerDesc& Desc) -> Entry&
    {
        NEON_ASSERT(m_Type == MaterialDataType::Sampler);
        auto& DescArray       = std::get<std::vector<RHI::SamplerDesc>>(m_DescriptorDesc);
        DescArray[ArrayIndex] = Desc;
        return *this;
    }

    auto MaterialMetaDataBuilder::Entry::SetDescriptor(
        size_t                         ArrayIndex,
        const RHI::DescriptorViewDesc& Desc) -> Entry&
    {
        NEON_ASSERT(
            m_Type != MaterialDataType::Constants &&
            m_Type != MaterialDataType::Sampler);
        auto& DescArray       = std::get<std::vector<RHI::DescriptorViewDesc>>(m_DescriptorDesc);
        DescArray[ArrayIndex] = Desc;
        return *this;
    }

    auto MaterialMetaDataBuilder::Entry::PreferDescriptor(
        bool Enable) -> Entry&
    {
        if ((m_PreferDescriptor = Enable))
        {
            if (m_Type != MaterialDataType::Sampler)
            {
                m_DescriptorDesc.emplace<std::vector<RHI::DescriptorViewDesc>>(m_ArraySize);
            }
            else
            {
                m_DescriptorDesc.emplace<std::vector<RHI::SamplerDesc>>(m_ArraySize);
            }
        }
        return *this;
    }

    MaterialMetaDataBuilder::Entry::Entry(
        StringU8         Name,
        MaterialDataType Type,
        size_t           ArraySize,
        size_t           DataSize) :
        m_Name(std::move(Name)),
        m_DataSize(uint32_t(DataSize)),
        m_ArraySize(uint16_t(ArraySize)),
        m_Type(Type)
    {
    }

    //

    MaterialMetaDataBuilder::Entry& MaterialMetaDataBuilder::AppendConstant(
        StringU8 Name,
        size_t   DataSize,
        size_t   ArraySize)
    {
        return m_Entries.emplace_back(std::move(Name), MaterialDataType::Constants, ArraySize, Math::AlignUp(DataSize, sizeof(int)));
    }

    MaterialMetaDataBuilder::Entry& MaterialMetaDataBuilder::AppendResource(
        StringU8         Name,
        MaterialDataType Type,
        size_t           ArraySize)
    {
        return m_Entries.emplace_back(std::move(Name), Type, ArraySize, 0);
    }
} // namespace Neon::Renderer
