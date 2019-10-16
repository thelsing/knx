#ifdef ARDUINO_ARCH_SAMD
#include <Arduino.h>
#include "samd_flash.h"

#define VALID      (0xDEADC0DE)

extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;

static const uint32_t pageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };

SamdFlash::SamdFlash(){
    _pageSize = pageSizes[NVMCTRL->PARAM.bit.PSZ];
    _pageCnt = NVMCTRL->PARAM.bit.NVMP;
    _rowSize = PAGES_PER_ROW * _pageSize;
    _rowBuffer = new uint8_t[_rowSize];

    //find end of program flash and set limit to next row
    uint32_t endEddr = (uint32_t)(&__etext + (&__data_end__ - &__data_start__));        //text + data MemoryBlock
    _MemoryEnd = getRowAddr(endEddr) + _rowSize-1;                        //23295

    //map info structure to last row in flash
    _info = (info_data_t*)getRowAddr(_pageSize*_pageCnt-1);     //261888 (rowAddr of last row)
}

uint8_t* SamdFlash::getStartAddress(){
    return (uint8_t*)_info->freeMemoryStart;
}

uint8_t SamdFlash::read(uint8_t* addr){
    if(_rowBufferModified == false)
      return *addr;

    if(getRowAddr((uint32_t)addr) != _rowBufferAddr)
      return *addr;

    //return data from buffer because flash data is not up to date
    return _rowBuffer[(uint32_t)addr-_rowBufferAddr];
}

bool SamdFlash::write(uint8_t* addr, uint8_t data){
    //Check if the destination address is valid
    if ((uint32_t)addr >= (_pageSize * _pageCnt) || (uint32_t)addr <= _MemoryEnd)
        return false;

    uint32_t newRowAddr = getRowAddr((uint32_t)addr);
    //if the row changed, write old Buffer and init new
    if(newRowAddr != _rowBufferAddr){
        uploadRowBuffer(_rowBufferAddr);
        downloadRowBuffer(newRowAddr);
        _rowBufferAddr = newRowAddr;
    }

    _rowBuffer[(uint32_t)addr-_rowBufferAddr] = data;
    _rowBufferModified = true;
    return true;
}

void SamdFlash::uploadRowBuffer(uint32_t rowAddr){
    if(!_rowBufferModified) return;
    eraseRow(rowAddr);
    //Clear Page Buffer
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
    while (NVMCTRL->INTFLAG.bit.READY == 0) {}
    // Disable automatic page write
    NVMCTRL->CTRLB.bit.MANW = 1;

    volatile uint32_t* src_addr = (volatile uint32_t*)_rowBuffer;
    volatile uint32_t* dst_addr = (volatile uint32_t *)rowAddr;
    for(uint32_t p=0;p<PAGES_PER_ROW;p++){
        for (uint32_t i=0; i<(_pageSize/4); i++) {
            *dst_addr = *src_addr;
            dst_addr++;
            src_addr++;
        }
        // Execute "WP" Write Page
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
        while (NVMCTRL->INTFLAG.bit.READY == 0) { }
    }
}
void SamdFlash::downloadRowBuffer(uint32_t rowAddr){
    volatile uint32_t* src_addr = (volatile uint32_t*)rowAddr;
    volatile uint32_t* dst_addr = (volatile uint32_t *)_rowBuffer;
    for(uint32_t p=0;p<PAGES_PER_ROW;p++){
        for (uint32_t i=0; i<(_pageSize/4); i++) {
            *dst_addr = *src_addr;
            dst_addr++;
            src_addr++;
        }
    }
    _rowBufferModified = false;
}

uint32_t SamdFlash::getRowAddr(uint32_t flasAddr){
    return flasAddr & ~(_rowSize-1);
}

void SamdFlash::finalise(){
    uploadRowBuffer(_rowBufferAddr);
}

void SamdFlash::eraseRow(uint32_t rowAddr){
    NVMCTRL->ADDR.reg = rowAddr/2;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
    while (!NVMCTRL->INTFLAG.bit.READY) { }
}

void SamdFlash::erase(){
    uint32_t rowAddr = getRowAddr((uint32_t)_info);
    uint32_t nextrowAddr = getRowAddr(_MemoryEnd);

    while(rowAddr != nextrowAddr){
        eraseRow(rowAddr);
        rowAddr = rowAddr - _rowSize;
    }
}

void SamdFlash::erase(uint32_t ID){
    block_meta_data_t ramCopy;
    block_meta_data_t* block = getBlock(ID);
    if(block == NULL)
        return;

    ramCopy = *block;
    uint32_t rowAddr = (uint32_t)block;
    uint32_t nextrowAddr = getRowAddr(rowAddr + block->size - 1 +sizeof(block_meta_data_t));

    while(rowAddr != nextrowAddr){
        eraseRow(nextrowAddr);
        nextrowAddr = nextrowAddr - _rowSize;
    }
    uploadRowBuffer(_rowBufferAddr);

    _rowBufferAddr = rowAddr;
    downloadRowBuffer(_rowBufferAddr);
    //map block meta structure into ram copy
    block = (block_meta_data_t*)_rowBuffer;
    //copy into row buffer
    *block = ramCopy;
    _rowBufferModified = true;
    uploadRowBuffer(_rowBufferAddr);
}
void SamdFlash::copyAndFree(block_meta_data_t* src, block_meta_data_t* dst){
    block_meta_data_t metaDataRamCopy;
    block_meta_data_t* tempBlock;
    if(src == NULL || dst == NULL)
        return;

    metaDataRamCopy = *dst;
    uint32_t lastRow = getRowAddr((uint32_t)src + src->size -1 + sizeof(block_meta_data_t));
    uint32_t srcRow = (uint32_t)src;
    uint32_t dstRow = (uint32_t)dst;

    while(srcRow <= lastRow){
        downloadRowBuffer(srcRow);
        //if first row copy meta data too
        if(srcRow == (uint32_t)src){
            //map block meta structure into ram copy
            tempBlock = (block_meta_data_t*)_rowBuffer;
            //copy into row buffer
            *tempBlock = metaDataRamCopy;
        }
        _rowBufferModified = true;
        uploadRowBuffer(dstRow);

        srcRow = srcRow + _rowSize;
        dstRow = dstRow + _rowSize;
    }

    //free src
    downloadRowBuffer((uint32_t)src);
    //map block meta structure into ram copy
    tempBlock = (block_meta_data_t*)_rowBuffer;
    tempBlock->ID = 0;
    _rowBufferModified = true;
    uploadRowBuffer((uint32_t)src);
}

block_meta_data_t* SamdFlash::findEmptyBlock(uint32_t size,uint32_t* compSize, uint32_t* previousBlockAddr){
    block_meta_data_t* block = _info->firstBlock;
    block_meta_data_t* preblock = NULL;
    block_meta_data_t* conblock = NULL;
    if(block != NULL && block->validMask == VALID){
      do{
        if(block->ID == 0){
          if(block->size >= size){
            *compSize = 0;
            return block;
          }
          //ceck if we can merge next empty blocks
          else if(block->next != NULL){
            conblock = block->next;
            *compSize = block->size;
            while(conblock != NULL && conblock->ID == 0){
              *compSize += conblock->size + sizeof(block_meta_data_t);
              if(*compSize >= size){
                *previousBlockAddr = (uint32_t)preblock;
                return conblock;
              }
              conblock = conblock->next;
            }
          }
          //block is last one
          else{
              *compSize = 0;
              *previousBlockAddr = (uint32_t)preblock;
              return block;
          }
        }
        preblock = block;
        block = block->next;
      }while(block != NULL && block->validMask == VALID);
    }
    return NULL;
}

uint8_t* SamdFlash::loadBlock(uint32_t ID){
    block_meta_data_t* block = _info->firstBlock;
    if(block == (block_meta_data_t*)0xFFFFFFFF)
        return NULL;

    if(block != NULL && block->validMask == VALID){
      do{
        if(block->ID == ID){
          return ((uint8_t*)block)+sizeof(block_meta_data_t);
        }
        block = block->next;
      }while(block != NULL && block->validMask == VALID);
    }
    return NULL;
}

block_meta_data_t* SamdFlash::getBlock(uint32_t ID){
    uint8_t* block = loadBlock(ID);
    if(block == NULL)
        return NULL;
    return (block_meta_data_t*)(block-sizeof(block_meta_data_t));
}

block_meta_data_t* SamdFlash::findLastBlock(){
    block_meta_data_t* block = _info->firstBlock;
    if(block == (block_meta_data_t*)0xFFFFFFFF || block == NULL)
        return NULL;
    while(block->next != NULL && block->next->validMask == VALID){
        block = block->next;
    }
    return block;
}

void SamdFlash::free(uint32_t ID){
    block_meta_data_t* block = getBlock(ID);
    if(block != NULL){
        //upload actual rowBuffer if necessary
        uploadRowBuffer(_rowBufferAddr);
        //download block into ram buffer
        _rowBufferAddr = (uint32_t)block;
        downloadRowBuffer(_rowBufferAddr);
        //map block structure into ram copy
        block = (block_meta_data_t*)_rowBuffer;
        //mark the block as empty and write to flash
        block->ID = 0;
        _rowBufferModified = true;
        uploadRowBuffer(_rowBufferAddr);
    }
}

uint8_t* SamdFlash::malloc(uint32_t size, uint32_t ID){
    //check if ID is already present
    if(loadBlock(ID) != NULL)
        return NULL;

    return forcemalloc(size, ID);

}

uint8_t* SamdFlash::forcemalloc(uint32_t size, uint32_t ID){
    uploadRowBuffer(_rowBufferAddr);
    //download actual info row into ram buffer
    _rowBufferAddr = (uint32_t)_info;            //261888
    downloadRowBuffer(_rowBufferAddr);
    //map info structure into ram copy
    _info = (info_data_t*)_rowBuffer; 
    
    //create new info if actual is not valid (all data got lost) 
    if(_info->validMask != VALID){
        _info->validMask = VALID;
        _info->freeMemoryStart = getRowAddr(_pageSize*_pageCnt-1)-1;   //161887
        _info->firstBlock = NULL;
    }

    //create ram copies for further usage
    block_meta_data_t newBlock;
    block_meta_data_t previousBlock;
    uint32_t newBlockFlashAddr;
    uint32_t previousBlockFlashAddr = 0;
    block_meta_data_t* tempBlock;

    //check if we can use an existing empty block or allocate a new one
    uint32_t newSize;
    tempBlock = findEmptyBlock(size,&newSize,&previousBlockFlashAddr);
    if(tempBlock != NULL){
        //copy found block into ram
        newBlock = *tempBlock;
        newBlockFlashAddr = (uint32_t)tempBlock;
        //two ore more empty blocks were connected
        if(newSize != 0){
            newBlock.size = newSize;
            if(previousBlockFlashAddr == 0){
                _info->firstBlock = (block_meta_data_t*)newBlockFlashAddr;
            }
            else{
                //copy found block into ram
                previousBlock = *((block_meta_data_t*)previousBlockFlashAddr);
                previousBlock.next = (block_meta_data_t*)newBlockFlashAddr;
            }
        }
        //newBlock is last one in list an can be extended
        if(newBlock.size < size){
            if(previousBlockFlashAddr == 0){
                newBlockFlashAddr = getRowAddr(getRowAddr(_pageSize*_pageCnt-1) - (size+sizeof(block_meta_data_t)));
                newBlock.size = getRowAddr(_pageSize*_pageCnt-1)-newBlockFlashAddr-sizeof(block_meta_data_t); //1008
                //update newBlockAddr in list
                _info->firstBlock = (block_meta_data_t*)newBlockFlashAddr;
            }
            else{
                newBlockFlashAddr = getRowAddr(previousBlockFlashAddr - (size+sizeof(block_meta_data_t)));
                newBlock.size = previousBlockFlashAddr-newBlockFlashAddr-sizeof(block_meta_data_t); //1008
                //copy found block into ram and update newBlockAddr in list
                previousBlock = *((block_meta_data_t*)previousBlockFlashAddr);
                previousBlock.next = (block_meta_data_t*)newBlockFlashAddr;
            }
            //set MemoryStart to end of new block
            _info->freeMemoryStart = newBlockFlashAddr-1; //260863
        }
        //fill meta data of new block
        newBlock.validMask = VALID;
        newBlock.ID = ID;
    }
    else{
        //check if size fits into free area
        if(_info->freeMemoryStart - (size+sizeof(block_meta_data_t)) <= _MemoryEnd)
            return NULL;

        //get start address of new block
        newBlockFlashAddr = getRowAddr(_info->freeMemoryStart +1 - (size+sizeof(block_meta_data_t)));   //260864 (size= 1024-16=1008)
        newBlock.size = _info->freeMemoryStart-newBlockFlashAddr+1-sizeof(block_meta_data_t); //1008
        newBlock.next = NULL;
        //set MemoryStart to end of new block
        _info->freeMemoryStart = newBlockFlashAddr-1; //260863
        //fill meta data of new block
        newBlock.validMask = VALID;
        newBlock.ID = ID;

        //add block to end of list
        tempBlock = findLastBlock();//(block_meta_data_t*)previousBlockFlashAddr;
        if(tempBlock == NULL){
            _info->firstBlock = (block_meta_data_t*)newBlockFlashAddr;
        }
        else{
            //copy found block into ram
            previousBlock = *tempBlock;
            previousBlock.next = (block_meta_data_t*)newBlockFlashAddr;
            previousBlockFlashAddr = (uint32_t)tempBlock;
        }
    }

    //write modified ram info structure into last flash row
    _rowBufferModified = true;
    uploadRowBuffer(_rowBufferAddr);

    //write modified ram copy of last block meta into flash
    if(previousBlockFlashAddr != 0){
        _rowBufferAddr = previousBlockFlashAddr;
        downloadRowBuffer(_rowBufferAddr);
        //map block meta structure into ram copy
        tempBlock = (block_meta_data_t*)_rowBuffer;
        //copy into row buffer
        *tempBlock = previousBlock;

        _rowBufferModified = true;
        uploadRowBuffer(_rowBufferAddr);
    }
    
    //write ram copy of new block meta into flash
    _rowBufferAddr = newBlockFlashAddr;
    downloadRowBuffer(_rowBufferAddr);
    //map block meta structure into ram copy
    tempBlock = (block_meta_data_t*)_rowBuffer;
    //copy into row buffer
    *tempBlock = newBlock;
    _rowBufferModified = true;
    uploadRowBuffer(_rowBufferAddr);
    
    //config structure now points into updated flash again
    _info = (info_data_t*)getRowAddr(_pageSize*_pageCnt-1);
    return (uint8_t*)(newBlockFlashAddr+sizeof(block_meta_data_t));
}

uint8_t* SamdFlash::realloc(uint32_t size, uint32_t ID){
    uint32_t newBlockFlashAddr;
    block_meta_data_t* actualBlock;

    actualBlock = getBlock(ID);

    //check if ID is presenty
    if(actualBlock == NULL)
        return malloc(size, ID);

    //if size already fits into block, nothing to do otherwise alloc block with new size
    if(actualBlock->size >= size)
        return loadBlock(ID);

    newBlockFlashAddr = (uint32_t)(forcemalloc(size, ID)-sizeof(block_meta_data_t));
    copyAndFree(actualBlock, (block_meta_data_t*)newBlockFlashAddr);
    return (uint8_t*)(newBlockFlashAddr+sizeof(block_meta_data_t));
}
#endif /* ARDUINO_ARCH_SAMD */
