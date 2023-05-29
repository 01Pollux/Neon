#pragma once

#include <RHI/RootSignature.hpp>
#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Commands/CommandList.hpp>
#include <RHI/Resource/Views/GenericView.hpp>
#include <Core/String.hpp>
#include <optional>
#include <vector>
#include <list>
#include <map>

namespace Neon::RHI
{
    class ISwapchain;
    class IGpuResource;
    class IBuffer;
    class ICommonCommandList;
} // namespace Neon::RHI

namespace Neon::Renderer
{
    class MaterialMetaDataBuilder;

    struct MaterialBinding
    {
        uint16_t Register = 0, Space = 0;
    };

    enum class MaterialDataType : uint8_t
    {
        Constants,
        ConstantBuffer,
        Resource,
        UavResource,
        Sampler
    };

    //

    class MaterialMetaData
    {
        using GraphicsResourcePtr  = Ptr<RHI::IGpuResource>;
        using GraphicsBufferPtr    = Ptr<RHI::IBuffer>;
        using GraphicsResourceList = std::vector<GraphicsResourcePtr>;

        using DescriptorHeapHandle = RHI::DescriptorHeapHandle;
        using IndirectResourceMap  = std::map<StringU8, std::pair<StringU8, uint32_t>>;

        //

        struct DescriptorEntry
        {
            std::vector<RHI::DescriptorViewDesc> Descs;
            std::vector<GraphicsResourcePtr>     Resources;

            uint32_t         Offset;
            MaterialDataType Type;
        };

        struct RootEntry
        {
            std::vector<GraphicsBufferPtr>    Resources;
            RHI::ICommonCommandList::ViewType Type;
        };

        struct ConstantEntry
        {
            std::vector<uint8_t> Data;
            size_t               DataStride;
        };

        struct SamplerEntry
        {
            std::vector<RHI::SamplerDesc> Descs;

            uint32_t Offset;
            uint32_t Count;
        };

        enum class EntryType : uint8_t
        {
            Descriptor,
            Root,
            Constant,
            Sampler
        };
        using EntryVariant = std::variant<DescriptorEntry, RootEntry, ConstantEntry, SamplerEntry>;

        struct EntryInLayout
        {
            EntryVariant Entry;
            uint32_t     RootIndex;
        };

    public:
        explicit MaterialMetaData(
            const MaterialMetaDataBuilder& Builder,
            RHI::ISwapchain*               SwapChain);

        MaterialMetaData() = default;

        MaterialMetaData(
            const MaterialMetaData& MetaData);
        MaterialMetaData& operator=(
            const MaterialMetaData& MetaData);

        MaterialMetaData(
            MaterialMetaData&& MetaData) noexcept;
        MaterialMetaData& operator=(
            MaterialMetaData&& MetaData) noexcept;

        ~MaterialMetaData() = default;

    private:
        /// <summary>
        /// Helper function for allocating descriptors
        /// </summary>
        void AllocateDescriptors(
            uint32_t ResourceCount,
            uint32_t SamplerCount);

        /// <summary>
        /// Helper function for destroying descriptors
        /// </summary>
        void DestroyDescriptors();

    public:
        /// <summary>
        /// Update material's data to command list
        /// </summary>
        void Update(
            RHI::ICommonCommandList* CommandList);

        /// <summary>
        /// Get root signature for material
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IRootSignature>& GetRootSignature() const;

    public:
        /// <summary>
        /// Set constants data or to constant buffer in the root signature
        /// </summary>
        void SetConstants(
            const StringU8& Name,
            size_t          ArrayIndex,
            size_t          Offset,
            const void*     Data,
            size_t          DataSize);

        /// <summary>
        /// Set resource and view in the root signature
        /// </summary>
        void SetResource(
            const StringU8&                               Name,
            size_t                                        ArrayIndex,
            const GraphicsResourcePtr&                    Resource,
            const std::optional<RHI::DescriptorViewDesc>& Desc);

    public:
        /// <summary>
        /// Set constants data or to constant buffer in the root signature
        /// </summary>
        void GetConstants(
            const StringU8& Name,
            size_t          ArrayIndex,
            size_t          Offset,
            void*           Data,
            size_t          DataSize) const;

        /// <summary>
        /// Set resource and view in the root signature
        /// </summary>
        void GetResource(
            const StringU8&          Name,
            size_t                   ArrayIndex,
            GraphicsResourcePtr*     Resource,
            RHI::DescriptorViewDesc* Desc) const;

    private:
        /// <summary>
        /// Set constants data or to constant buffer in the root signature
        /// </summary>
        void SetConstants_Internal(
            const StringU8& Name,
            size_t          ArrayIndex,
            size_t          ByteOffset,
            const void*     Data,
            size_t          DataSize);

        /// <summary>
        /// Set resource in the root signature
        /// </summary>
        void SetResource_Internal(
            const StringU8&                               Name,
            size_t                                        ArrayIndex,
            const GraphicsResourcePtr&                    Resource,
            const std::optional<RHI::DescriptorViewDesc>& Desc);

    private:
        /// <summary>
        /// Set constants data or to constant buffer in the root signature
        /// </summary>
        void GetConstants_Internal(
            const StringU8& Name,
            size_t          ArrayIndex,
            size_t          ByteOffset,
            void*           Data,
            size_t          DataSize) const;

        /// <summary>
        /// Set resource in the root signature
        /// </summary>
        void GetResource_Internal(
            const StringU8&          Name,
            size_t                   ArrayIndex,
            GraphicsResourcePtr*     Resource,
            RHI::DescriptorViewDesc* Desc) const;

    private:
        RHI::ISwapchain*                  m_SwapChain = nullptr;
        Ptr<RHI::IRootSignature>          m_RootSignature;
        IndirectResourceMap               m_IndirectResources;
        std::map<StringU8, EntryInLayout> m_LayoutMap;

        RHI::Views::Generic m_ResourceDescriptor;
        RHI::Views::Generic m_SamplerDescriptor;
    };

    //

    class MaterialMetaDataBuilder
    {
        friend class MaterialMetaData;

    public:
        class Entry
        {
            friend class MaterialMetaDataBuilder;
            friend class MaterialMetaData;

            using DescriptorDescType = std::variant<
                std::vector<RHI::SamplerDesc>,
                std::vector<RHI::DescriptorViewDesc>>;

        public:
            /// <summary>
            /// Set shader visibility for this entry
            /// </summary>
            Entry& SetShader(
                RHI::ShaderVisibility Visibility);

            /// <summary>
            /// Set bind space for material
            /// </summary>
            Entry& SetBinding(
                uint16_t Register,
                uint16_t Space = 0);

            /// <summary>
            /// Reference a material's data with a custom name
            /// </summary>
            Entry& SetReference(
                const StringU8& Name,
                uint32_t        ArrayIndex = 0);

            /// <summary>
            /// Set sampler descriptor for this entry (only for sampler)
            /// </summary>
            Entry& SetDescriptor(
                size_t                  ArrayIndex,
                const RHI::SamplerDesc& Desc);

            /// <summary>
            /// Set resource descriptor for this entry (only for resource)
            /// </summary>
            Entry& SetDescriptor(
                size_t                         ArrayIndex,
                const RHI::DescriptorViewDesc& Desc);

            /// <summary>
            /// Use descriptor or resource view for this entry
            /// </summary>
            Entry& PreferDescriptor(
                bool Enable);

            Entry(
                StringU8         Name,
                MaterialDataType Type,
                size_t           ArraySize,
                size_t           DataSize);

        private:
            StringU8                                   m_Name;
            std::vector<std::pair<StringU8, uint32_t>> m_References;

            DescriptorDescType    m_DescriptorDesc;
            RHI::ShaderVisibility m_Visiblity = RHI::ShaderVisibility::All;

            uint32_t        m_DataSize;
            MaterialBinding m_Binding;
            uint16_t        m_ArraySize;

            MaterialDataType m_Type;
            bool             m_PreferDescriptor : 1 = false;
        };

        /// <summary>
        /// Add a new entry to layout
        /// </summary>
        Entry& AppendConstant(
            StringU8 Name,
            size_t   DataSize,
            size_t   ArraySize = 1);

        /// <summary>
        /// Add a new entry to layout
        /// </summary>
        Entry& AppendResource(
            StringU8         Name,
            MaterialDataType Type,
            size_t           ArraySize = 1);

    private:
        std::list<Entry> m_Entries;
    };
} // namespace Neon::Renderer