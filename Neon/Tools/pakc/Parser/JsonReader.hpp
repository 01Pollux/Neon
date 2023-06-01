#pragma once

#include <boost/function.hpp>
#include <boost/json.hpp>

namespace PakC
{
    class JsonHandler
    {
    public:
        JsonHandler();

        /// <summary>
        /// Parses a JSON file.
        /// </summary>
        void Parse(
            const std::string& Path);

    private:
        void RegisterTypes();

    private:
        std::map<std::string, boost::function<void(const boost::json::value&)>> m_TypeMap;
    };
} // namespace PakC