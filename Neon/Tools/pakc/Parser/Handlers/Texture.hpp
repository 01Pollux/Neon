#pragma once

#include <Parser/JsonReader.hpp>

namespace PakC::Handler
{
    AssetResourcePtr LoadTextureResource(
        const boost::json::object& Object);
}