#pragma once

#include <Parser/JsonReader.hpp>

namespace PakC::Handler
{
    AssetResourcePtr LoadTextResource(
        const boost::json::object& Object);
}