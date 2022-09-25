#pragma once
#include <stdint.h>
#include <vector>

struct Instruction{
    uint8_t opcode;
    std::vector<float> params;

    Instruction()
    {
        params.resize(10);
    }
};