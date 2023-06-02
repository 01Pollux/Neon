#include <PakCPCH.hpp>
#include <Parser/Handlers/TextFile.hpp>
#include <fstream>

#include <Resource/Types/TextFile.hpp>
#include <Log/Logger.hpp>

namespace PakC::Handler
{
    AssetResourcePtr LoadTextResource(
        const boost::json::object& Object)
    {
        if (auto FileName = Object.find("File"); FileName != Object.end() && FileName->value().is_string())
        {
            std::string    Path(FileName->value().as_string());
            std::wifstream File(Path);
            if (!File)
            {
                throw std::runtime_error("Failed to open text file.");
            }
            std::wstringstream Stream;
            Stream << File.rdbuf();
            return std::make_shared<Neon::Asset::TextFileAsset>(Stream.str());
        }
        else
        {
            if (auto Content = Object.find("Content"); Content != Object.end() && Content->value().is_string())
            {
                return std::make_shared<Neon::Asset::TextFileAsset>(std::string(Content->value().as_string()));
            }
        }
        throw std::runtime_error("No file or content specified for text file.");
    }
} // namespace PakC::Handler