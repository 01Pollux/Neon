#pragma once

#include <Core/String.hpp>
#include <boost/property_tree/ptree.hpp>
#include <any>

namespace Neon::Renderer
{
    template<typename _Ty>
    struct BaseMaterialCustomVariable
    {
        using internal_type = std::string;
        using external_type = _Ty;

        using internal_type_opt = boost::optional<internal_type>;
        using external_type_opt = boost::optional<external_type>;
    };

    template<typename _Ty>
    struct MaterialCustomVariable : public BaseMaterialCustomVariable<_Ty>
    {
    };

    template<typename _Ty>
    concept HasMaterialCustomVariable = requires() {
        {
            MaterialCustomVariable<_Ty>::get_value(
                std::declval<typename BaseMaterialCustomVariable<_Ty>::internal_type>())
        } -> std::same_as<typename BaseMaterialCustomVariable<_Ty>::external_type_opt>;
        {
            std::declval<MaterialCustomVariable<_Ty>>().put_value(
                std::declval<typename BaseMaterialCustomVariable<_Ty>::external_type>())
        } -> std::same_as<typename BaseMaterialCustomVariable<_Ty>::internal_type_opt>;
    };

    //

    class MaterialProperty
    {
    public:
        using PropertyTreeType = boost::property_tree::ptree;

    public:
        MaterialProperty() = default;
        MaterialProperty(
            PropertyTreeType Prop) :
            m_Prop(std::move(Prop))
        {
        }

        class MaterialPropertyProxy
        {
        public:
            MaterialPropertyProxy(
                PropertyTreeType* Prop) :
                m_Prop(Prop)
            {
            }

            MaterialPropertyProxy(
                PropertyTreeType* Prop,
                const StringU8&   Name)
            {
                auto Iter = Prop->get_child_optional(Name);
                if (Iter)
                {
                    m_Prop = &*Iter;
                }
                else
                {
                    m_Prop = &Prop->put_child(Name, {});
                }
            }

        public:
            /// <summary>
            /// Get child property tree
            /// </summary>
            [[nodiscard]] MaterialPropertyProxy GetChild(
                const StringU8& Name)
            {
                return MaterialPropertyProxy(&m_Prop->get_child(Name));
            }

            /// <summary>
            /// Get child property tree if exists
            /// </summary>
            [[nodiscard]] boost::optional<MaterialPropertyProxy> GetChildOpt(
                const StringU8& Name)
            {
                boost::optional<MaterialPropertyProxy> Opt;
                if (auto Child = m_Prop->get_child_optional(Name))
                {
                    Opt = MaterialPropertyProxy(&*Child);
                }
                return Opt;
            }

            /// <summary>
            /// Set child property tree
            /// </summary>
            void SetChild(
                const StringU8&  Name,
                MaterialProperty Prop)
            {
                m_Prop->put_child(Name, std::move(Prop.m_Prop));
            }

        public:
            void erase(
                StringU8& Name)
            {
                m_Prop->erase(Name);
            }

            void erase(
                PropertyTreeType::iterator Iter)
            {
                m_Prop->erase(Iter);
            }

            void erase(
                PropertyTreeType::iterator First,
                PropertyTreeType::iterator Last)
            {
                m_Prop->erase(First, Last);
            }

            [[nodiscard]] auto begin()
            {
                return m_Prop->begin();
            }

            [[nodiscard]] auto end()
            {
                return m_Prop->end();
            }

            [[nodiscard]] auto begin() const
            {
                return m_Prop->begin();
            }

            [[nodiscard]] auto end() const
            {
                return m_Prop->end();
            }

            [[nodiscard]] auto size() const
            {
                return m_Prop->size();
            }

            [[nodiscard]] auto find(
                const StringU8& Name)
            {
                return m_Prop->find(Name);
            }

            [[nodiscard]] auto find(
                const StringU8& Name) const
            {
                return m_Prop->find(Name);
            }

            [[nodiscard]] auto not_found() const
            {
                return m_Prop->not_found();
            }

            [[nodiscard]] bool contains(
                const StringU8& Name) const
            {
                return find(Name) != not_found();
            }

        public:
            /// <summary>
            /// Get value from property tree
            /// </summary>
            template<typename _Ty>
            [[nodiscard]] _Ty Get()
            {
                if constexpr (HasMaterialCustomVariable<_Ty>)
                {
                    return m_Prop->get_value<_Ty>(MaterialCustomVariable<_Ty>());
                }
                else
                {
                    return m_Prop->get_value<_Ty>();
                }
            }

            /// <summary>
            /// Get value from property tree
            /// </summary>
            template<typename _Ty>
            [[nodiscard]] _Ty Get(
                const _Ty& Default)
            {
                if constexpr (HasMaterialCustomVariable<_Ty>)
                {
                    return m_Prop->get_value<_Ty>(Default, MaterialCustomVariable<_Ty>());
                }
                else
                {
                    return m_Prop->get_value<_Ty>(Default);
                }
            }

            /// <summary>
            /// Get value from property tree
            /// </summary>
            template<typename _Ty>
            [[nodiscard]] boost::optional<_Ty> GetOpt()
            {
                if constexpr (HasMaterialCustomVariable<_Ty>)
                {
                    return m_Prop->get_value_optional<_Ty>(MaterialCustomVariable<_Ty>());
                }
                else
                {
                    return m_Prop->get_value_optional<_Ty>();
                }
            }

            /// <summary>
            /// Set value to property tree
            /// </summary>
            template<typename _Ty>
            void Set(
                const _Ty& Value)
            {
                if constexpr (HasMaterialCustomVariable<_Ty>)
                {
                    m_Prop->put_value(Value, MaterialCustomVariable<_Ty>());
                }
                else
                {
                    m_Prop->put_value(Value);
                }
            }

        private:
            PropertyTreeType* m_Prop = nullptr;
        };

        using Proxy = MaterialPropertyProxy;

    public:
        /// <summary>
        /// Get child property tree
        /// </summary>
        [[nodiscard]] MaterialPropertyProxy GetChild(
            const StringU8& Name)
        {
            return MaterialPropertyProxy(&m_Prop).GetChild(Name);
        }

        /// <summary>
        /// Get child property tree if exists
        /// </summary>
        [[nodiscard]] boost::optional<MaterialPropertyProxy> GetChildOpt(
            const StringU8& Name)
        {
            return MaterialPropertyProxy(&m_Prop).GetChildOpt(Name);
        }

        /// <summary>
        /// Set child property tree
        /// </summary>
        void SetChild(
            const StringU8&  Name,
            MaterialProperty Prop)
        {
            MaterialPropertyProxy(&m_Prop).SetChild(Name, std::move(Prop));
        }

    public:
        void erase(
            StringU8& Name)
        {
            m_Prop.erase(Name);
        }

        void erase(
            PropertyTreeType::iterator Iter)
        {
            m_Prop.erase(Iter);
        }

        void erase(
            PropertyTreeType::iterator First,
            PropertyTreeType::iterator Last)
        {
            m_Prop.erase(First, Last);
        }

        [[nodiscard]] auto begin()
        {
            return m_Prop.begin();
        }

        [[nodiscard]] auto end()
        {
            return m_Prop.end();
        }

        [[nodiscard]] auto begin() const
        {
            return m_Prop.begin();
        }

        [[nodiscard]] auto end() const
        {
            return m_Prop.end();
        }

        [[nodiscard]] auto size() const
        {
            return m_Prop.size();
        }

        [[nodiscard]] auto find(
            const StringU8& Name)
        {
            return m_Prop.find(Name);
        }

        [[nodiscard]] auto find(
            const StringU8& Name) const
        {
            return m_Prop.find(Name);
        }

        [[nodiscard]] auto not_found() const
        {
            return m_Prop.not_found();
        }

        [[nodiscard]] bool contains(
            const StringU8& Name) const
        {
            return find(Name) != not_found();
        }

    public:
        /// <summary>
        /// Get value from property tree
        /// </summary>
        template<typename _Ty>
        [[nodiscard]] _Ty Get(
            const StringU8& Name)
        {
            return MaterialPropertyProxy(&m_Prop, Name).Get<_Ty>();
        }

        /// <summary>
        /// Get value from property tree
        /// </summary>
        template<typename _Ty>
        [[nodiscard]] _Ty Get(
            const StringU8& Name,
            const _Ty&      Default)
        {
            return MaterialPropertyProxy(&m_Prop, Name).Get<_Ty>(Default);
        }

        /// <summary>
        /// Get value from property tree
        /// </summary>
        template<typename _Ty>
        [[nodiscard]] boost::optional<_Ty> GetOpt(
            const StringU8& Name)
        {
            return MaterialPropertyProxy(&m_Prop, Name).GetOpt<_Ty>();
        }

        /// <summary>
        /// Set value to property tree
        /// </summary>
        template<typename _Ty>
        void Set(
            const StringU8& Name,
            const _Ty&      Value)
        {
            MaterialPropertyProxy(&m_Prop, Name).Set(Value);
        }

    private:
        PropertyTreeType m_Prop;
    };
} // namespace Neon::Renderer
