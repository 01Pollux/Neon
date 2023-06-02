#pragma once

#include <Parser/JsonReader.hpp>

namespace PakC::Handler
{
    AssetResourcePtr LoadLoggerResource(
        const boost::json::object& Object);
}