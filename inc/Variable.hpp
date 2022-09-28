#pragma once

#include <string>

namespace variable
{
    struct Numeric
    {
        uint16_t name = 0;
        double value = 0 ;
    };

    struct String
    {
        uint16_t name = 0;
        char* value = 0;
        uint16_t size = 0;
    };

    struct Target
    {
        uint16_t name = 0;
        float angles[6];

    };
}