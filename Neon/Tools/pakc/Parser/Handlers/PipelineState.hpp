#pragma once

#include <Parser/JsonReader.hpp>

namespace PakC::Handler
{
    AssetResourcePtr LoadPipelineStateResource(
        const boost::json::object& Object);
}