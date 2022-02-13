#pragma once
#include <stdint.h>
#include <vector>

struct Instruction{
    uint8_t opcode;
    int32_t params[10];
};