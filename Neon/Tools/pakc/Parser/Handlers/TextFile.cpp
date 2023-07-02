#include <PakCPCH.hpp>
#include <Parser/Handlers/TextFile.hpp>
#include <fstream>

#include <Resource/Types/TextFile.hpp>
#include <Log/Logger.hpp>

namespace PakC::Handler
{
    using namespace Neon;
    AssetResourcePtr LoadTextResource(
        const boost::json::object& Object)
    {
        if (auto FileName = Object.if_contains("File"); FileName && FileName->is_string())
        {
            StringU8       Path(FileName->as_string());
            std::wifstream File(Path);
            if (!File)
            {
                throw std::runtime_error("Failed to open text file.");
            }
            std::wstringstream Stream;
            Stream << File.rdbuf();
            return std::make_shared<Asset::TextFileAsset>(Stream.str());
        }
        else if (auto Content = Object.if_contains("Content"); Content && Content->is_string())
        {
            return std::make_shared<Asset::TextFileAsset>(StringU8(Content->as_string()));
        }
        else
        {
            throw std::runtime_error("No file or content specified for text file.");
        }
    }
} // namespace PakC::Handler