//#pragma once
#include <cstdint>
#include <vector>

struct Instruction{
    uint8_t opcode;
    std::vector<float> params;

    Instruction()
    {
        params.resize(10);
        opcode = 0;
    }
};