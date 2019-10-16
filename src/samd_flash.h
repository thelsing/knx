#ifndef SAMD_FLASH_H
#define SAMD_FLASH_H

#ifdef ARDUINO_ARCH_SAMD
#include <Arduino.h>

#define PAGES_PER_ROW	4

typedef struct block_meta_data{
    uint32_t validMask;
    uint32_t ID;
    uint32_t size;
    struct block_meta_data *next;
}__attribute__((__packed__))block_meta_data_t;

typedef struct {
    uint32_t validMask;
    uint32_t freeMemoryStart;
    block_meta_data_t* firstBlock;
}__attribute__((__packed__))info_data_t;

class SamdFlash{
public:
    SamdFlash();
    void erase();
    void erase(uint32_t ID);
    void free(uint32_t ID);
    uint8_t* malloc(uint32_t size, uint32_t ID);
    uint8_t* realloc(uint32_t size, uint32_t ID);
    uint8_t* loadBlock(uint32_t ID);
    bool write(uint8_t* addr, uint8_t data);
    uint8_t read(uint8_t* addr);
    void finalise();
    uint8_t* getStartAddress();

private:
    uint8_t* forcemalloc(uint32_t size, uint32_t ID);
    void uploadRowBuffer(uint32_t rowAddr);
    void downloadRowBuffer(uint32_t rowAddr);
    uint32_t getRowAddr(uint32_t flasAddr);
    void eraseRow(uint32_t rowAddr);
    void copyAndFree(block_meta_data_t* src, block_meta_data_t* dst);


    block_meta_data_t* findEmptyBlock(uint32_t size,uint32_t* compSize, uint32_t* nextBlockAddr);
    block_meta_data_t* findLastBlock();
    block_meta_data_t* getBlock(uint32_t ID);

    uint32_t _MemoryEnd=0;
    uint32_t _pageSize;
    uint32_t _rowSize;
    uint32_t _pageCnt;


    uint8_t* _rowBuffer;
    bool _rowBufferModified = false;
    uint32_t _rowBufferAddr;
    volatile info_data_t* _info;
};
#endif /* ARDUINO_ARCH_SAMD */
#endif /* SAMD_FLASH_H */
