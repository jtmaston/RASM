#pragma once
#include <stdint.h>
#include <vector>

struct Instruction{
    uint8_t opcode;
    float params[10];
};