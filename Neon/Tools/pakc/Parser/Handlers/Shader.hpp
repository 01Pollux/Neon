#pragma once

#include <Parser/JsonReader.hpp>

namespace PakC::Handler
{
    AssetResourcePtr LoadShaderResource(
        const boost::json::object& Object);
}