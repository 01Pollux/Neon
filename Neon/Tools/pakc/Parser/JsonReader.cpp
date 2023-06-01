#include <PakCPCH.hpp>
#include <Parser/JsonReader.hpp>
#include <fstream>
#include <iostream>

#include <Log/Logger.hpp>

namespace PakC
{
    JsonHandler::JsonHandler()
    {
        RegisterTypes();
    }

    void JsonHandler::Parse(
        const std::string& Path)
    {
        NEON_INFO("Parsing: '{}'", Path);

        std::ifstream File(Path);
        if (!File)
        {
            NEON_ERROR("Failed to open file");
            return;
        }

        auto Json = boost::json::parse(File);
        File.close();

        size_t Count = 0;
        for (auto& Element : Json.as_array() |
                                 std::views::transform([](auto& Element) -> decltype(auto)
                                                       { return Element.as_object(); }))
        {
            Count++;
            auto Type = Element.find("Type");
            if (Type == Element.end() || !Type->value().is_string())
            {
                NEON_INFO("Failed to find type at position: {}", Count);
                continue;
            }

            auto Tag = Element.find("Tag");
            if (Tag != Element.end() && Tag->value().is_string())
            {
                NEON_INFO("Loading: '{}'", Tag->value().as_string());
            }

            auto TypeHandler = m_TypeMap.find(std::string(Type->value().as_string()));
            if (TypeHandler == m_TypeMap.end())
            {
                NEON_INFO("Failed to find handler for type: '{}'", Type->value().as_string());
                continue;
            }
        }
    }

    void JsonHandler::RegisterTypes()
    {
        m_TypeMap["TextFileAsset"] = [](const boost::json::value& Value)
        {
            std::cout << "Type: " << Value.as_string() << std::endl;
        };
    }
} // namespace PakC