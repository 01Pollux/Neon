#pragma once

#include <RHI/RootSignature.hpp>
#include <RHI/CommandList.hpp>
#include <RHI/Sampler.hpp>

namespace Renderer
{
    enum class ShaderVisibility : uint8_t
    {
        All,
        Compute = All,
        Vertex,
        Hull,
        Domain,
        Geometry,
        Pixel,
    };

    //

    struct MaterialBinding
    {
        uint16_t Register = 0, Space = 0;
    };

    enum class MaterialDataType : uint8_t
    {
        Unknown,
        Constants,
        ConstantBuffer,
        Resource,
        UavResource,
        Sampler
    };

    //

    class MaterialMetaDataBuilder;

    class MaterialMetaData
    {
        using GraphicsResourcePtr    = RHI::GraphicsResource::Ptr;
        using GraphicsResourceViewId = RHI::GraphicsResource::ViewId;
        using GraphicsResourceList   = std::vector<GraphicsResourcePtr>;

        using DescriptorHeapHandle = RHI::IDescriptorHeapAllocator::Handle;
        using IndirectResourceMap  = std::map<StringU8, std::pair<StringU8, uint32_t>>;

        //

        struct DescriptorEntry
        {
            std::vector<RHI::DescriptorViewDesc>    Descs;
            std::vector<RHI::GraphicsResource::Ptr> Resources;

            uint32_t         Offset;
            MaterialDataType Type;
        };

        struct RootEntry
        {
            std::vector<RHI::GraphicsBuffer::Ptr> Resources;

            RHI::ResourceViewType ViewType;
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
            const MaterialMetaDataBuilder& Builder);

        MaterialMetaData() = default;

        MaterialMetaData(
            const MaterialMetaData& MetaData);
        MaterialMetaData& operator=(
            const MaterialMetaData& MetaData);

        MaterialMetaData(
            MaterialMetaData&& MetaData) noexcept;
        MaterialMetaData& operator=(
            MaterialMetaData&& MetaData) noexcept;

        ~MaterialMetaData();

    private:
        /// <summary>
        /// Helper function for copy op and constructor
        /// </summary>
        void AllocateDescriptors();

        /// <summary>
        /// Helper function for move assign op and desctructor
        /// </summary>
        void DestroyDescriptors() const;

    public:
        /// <summary>
        /// Update material's data to command list
        /// </summary>
        void Update(
            RHI::ICommonCommandContext& Context);

        /// <summary>
        /// Get root signature for material
        /// </summary>
        [[nodiscard]] const RHI::RootSignature& GetRootSignature() const;

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
        RHI::RootSignature                m_RootSignature;
        IndirectResourceMap               m_IndirectResources;
        std::map<StringU8, EntryInLayout> m_LayoutMap;

        DescriptorHeapHandle m_ResourceDescriptor;
        DescriptorHeapHandle m_SamplerDescriptor;
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

            using DescriptorDescType = std::variant<std::vector<RHI::SamplerDesc>, std::vector<RHI::DescriptorViewDesc>>;

        public:
            /// <summary>
            /// Set shader visibility for this entry
            /// </summary>
            Entry& SetShader(
                ShaderVisibility Visibility);

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

            DescriptorDescType      m_DescriptorDesc;
            D3D12_SHADER_VISIBILITY m_Visiblity = D3D12_SHADER_VISIBILITY_ALL;

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
} // namespace Renderer