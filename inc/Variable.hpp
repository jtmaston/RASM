#pragma once

#include <string>

namespace Variable
{
    struct Numeric
    {
        uint16_t name = 0;
        double value = 0;
    };

    struct String
    {
        // uint16_t name = 0;
        char *value = nullptr;
        uint16_t size = 0;
    };

    struct Target
    {
        uint16_t name = 0;
        std::array<float, 6> angles = {0};

    };
}