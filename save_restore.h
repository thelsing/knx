#pragma once
#include <stdint.h>


class SaveRestore
{
public:
    virtual uint8_t* save(uint8_t* buffer) = 0;
    virtual uint8_t* restore(uint8_t* buffer) = 0;
};