#include "platform.h"

#include <cstdlib>

uint8_t* Platform::memoryReference()
{
    return _memoryReference;
}


uint8_t* Platform::allocMemory(size_t size)
{
    uint8_t* address =  (uint8_t*)malloc(size);
    if (_memoryReference == 0)
        _memoryReference = address;
    
    return address;
}

void Platform::freeMemory(uint8_t* ptr)
{
    free(ptr);
}
