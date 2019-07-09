#include "platform.h"
#include "knx/bits.h"

#include <cstdlib>

uint8_t* Platform::memoryReference()
{
    return _memoryReference;
}


uint8_t* Platform::allocMemory(size_t size)
{
    uint8_t* address =  (uint8_t*)malloc(size);
//    if (_memoryReference == 0 || address < _memoryReference)
//        _memoryReference = address;
	print("MemRef: ");
	print((long unsigned int)_memoryReference, HEX);    
	print(" Mem: ");
	print((long unsigned int)address, HEX);
	print(" Diff: ");
	println((long unsigned int)(address - _memoryReference));
    return address;
}

void Platform::freeMemory(uint8_t* ptr)
{
    free(ptr);
}


Platform::Platform()
{
    // allocate memory to have a memory reference, substract a bit 
    _memoryReference = (uint8_t*)malloc(1)  - 1024;
    free(_memoryReference);
	print("MemRef: ");
	println((long unsigned int)_memoryReference, HEX);
}
