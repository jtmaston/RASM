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
}