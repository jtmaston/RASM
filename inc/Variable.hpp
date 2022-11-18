#pragma once

#include <string>
#include <vector>

namespace variable
{
    struct Numeric
    {
        uint16_t name;
        double value;
    };

    struct String
    {
        uint16_t name;
        char* value;
        uint16_t size;
    };

    struct Target
    {
        uint16_t name;
        std::vector<double> angles;

    };
}