#include "platform.h"
#include "knx/bits.h"

#include <cstdlib>

uint8_t* Platform::referenceNVMemory()
{
    return _memoryReference;
}


uint8_t* Platform::allocMemory(size_t size)
{
    uint8_t* address =  (uint8_t*)malloc(size);
    if(address == NULL)
        fatalError();
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
    _memoryReference = (uint8_t*)malloc(1);
    free(_memoryReference);
    _memoryReference -= 1024;
}

uint8_t Platform::popNVMemoryByte(uint8_t** addr)
{
    uint8_t val = readNVMemory(*addr);
    (*addr)+=1;
    return val;
}

uint16_t Platform::popNVMemoryWord(uint8_t** addr)
{
    uint16_t val = (readNVMemory(*addr) << 8) + readNVMemory((*addr)+1);
    (*addr)+=2;
    return val;
}

uint32_t Platform::popNVMemoryInt(uint8_t** addr)
{
    uint32_t val = (readNVMemory((*addr)) << 24) + (readNVMemory((*addr)+1) << 16) + (readNVMemory((*addr)+2) << 8) + readNVMemory((*addr)+3);
    (*addr)+=4;
    return val;
}

void Platform::popNVMemoryArray(uint8_t* dest, uint8_t** addr, size_t size){
    for(size_t i=0;i<size;i++){
        dest[i] = readNVMemory((*addr)++);
    }
}

void Platform::pushNVMemoryByte(uint8_t val, uint8_t** addr)
{
    writeNVMemory((*addr), val);
    (*addr) += 1;
}

void Platform::pushNVMemoryWord(uint16_t val, uint8_t** addr)
{
    writeNVMemory((*addr), ((val >> 8) & 0xff));
    writeNVMemory((*addr)+1, (val & 0xff));
    (*addr) += 2;
}

void Platform::pushNVMemoryInt(uint32_t val, uint8_t** addr)
{
    writeNVMemory((*addr), ((val >> 24) & 0xff));
    writeNVMemory((*addr)+1, ((val >> 16) & 0xff));
    writeNVMemory((*addr)+2, ((val >> 8) & 0xff));
    writeNVMemory((*addr)+3, (val & 0xff));
    (*addr) += 4;
}

void Platform::pushNVMemoryArray(uint8_t* src, uint8_t** addr, size_t size){
    for(size_t i=0;i<size;i++){
        writeNVMemory((*addr)++,src[i]);
    }
}
