#pragma once

#include <string>

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
        float angles[6];

    };
}