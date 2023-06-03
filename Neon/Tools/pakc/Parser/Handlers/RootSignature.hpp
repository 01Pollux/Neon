#pragma once

#include <Parser/JsonReader.hpp>

namespace PakC::Handler
{
    AssetResourcePtr LoadRootSignatureResource(
        const boost::json::object& Object);
}