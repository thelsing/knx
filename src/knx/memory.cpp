#include "memory.h"
#include <string.h>
#include "bits.h"

Memory::Memory(Platform& platform, DeviceObject& deviceObject)
    : _platform(platform), _deviceObject(deviceObject)
{
}

void Memory::readMemory()
{
    if (_data != nullptr)
        return;

    uint16_t flashSize = 512;
    _data = _platform.getEepromBuffer(flashSize);

    uint16_t metadataBlockSize = alignToPageSize(_metadataSize);

    _freeList = new MemoryBlock(_data + metadataBlockSize, flashSize - metadataBlockSize);

    uint16_t manufacturerId = 0;
    uint8_t* buffer = popWord(manufacturerId, _data);

    uint8_t hardwareType[LEN_HARDWARE_TYPE] = {0};
    buffer = popByteArray(hardwareType, LEN_HARDWARE_TYPE, buffer);

    uint16_t version = 0;
    buffer = popWord(version, buffer);
    
    if (_deviceObject.manufacturerId() != manufacturerId
       || _deviceObject.version() != version
       || memcmp(_deviceObject.hardwareType(), hardwareType, LEN_HARDWARE_TYPE) != 0)
    {
        println("saved memory doesn't match manufacturerId, version or hardwaretype");
        return;
    }

    for (int i = 0; i < _saveCount; i++)
    {
        buffer = _saveRestores[i]->restore(buffer);
    }

    for (int i = 0; i < _tableObjCount; i++)
    {
        buffer = _tableObjects[i]->restore(buffer);
        uint16_t memorySize = 0;
        buffer = popWord(memorySize, buffer);

        if (memorySize == 0)
            continue;

        // this works because TableObject saves a relative addr and restores it itself
        addNewUsedBlock(_tableObjects[i]->_data, memorySize);
    }
}

void Memory::writeMemory()
{
    uint8_t* buffer = _data;
    buffer = pushWord(_deviceObject.manufacturerId(), buffer);
    buffer = pushByteArray(_deviceObject.hardwareType(), LEN_HARDWARE_TYPE, buffer);
    buffer = pushWord(_deviceObject.version(), buffer);

    for (int i = 0; i < _saveCount; i++)
    {
        buffer = _saveRestores[i]->save(buffer);
    }

    for (int i = 0; i < _tableObjCount; i++)
    {
        buffer = _tableObjects[i]->save(buffer);

        //save to size of the memoryblock for tableobject too, so that we can rebuild the usedList and freeList
        if (_tableObjects[i]->_data != nullptr)
        {

            MemoryBlock* block = findBlockInList(_usedList, _tableObjects[i]->_data);
            if (block == nullptr)
            {
                println("_data of TableObject not in errorlist");
                _platform.fatalError();
            }
            buffer = pushWord(block->size, buffer);
        }
        else
            pushWord(0, buffer);
    }
    
    _platform.commitToEeprom();
}

void Memory::addSaveRestore(SaveRestore* obj)
{
    if (_saveCount >= MAXSAVE - 1)
        return;

    _saveRestores[_saveCount] = obj;
    _saveCount += 1;
    _metadataSize += obj->saveSize();
}

void Memory::addSaveRestore(TableObject* obj)
{
    if (_tableObjCount >= MAXTABLEOBJ)
        return;

    _tableObjects[_tableObjCount] = obj;
    _tableObjCount += 1;
    _metadataSize += obj->saveSize();
    _metadataSize += 2; // for size
}

uint8_t* Memory::allocMemory(size_t size)
{
    // always allocate aligned to 32 bit
    size = alignToPageSize(size);

    MemoryBlock* freeBlock = _freeList;
    MemoryBlock* blockToUse = nullptr;
    
    // find the smallest possible block that is big enough
    while (freeBlock)
    {
        if (freeBlock->size >= size)
        {
            if (blockToUse != nullptr && (blockToUse->size - size) > (freeBlock->size - size))
                blockToUse = freeBlock;
            else if (blockToUse == nullptr)
                blockToUse = freeBlock;
        }
        freeBlock = freeBlock->next;
    }
    if (!blockToUse)
    {
        println("No available non volatile memory!");
        _platform.fatalError();
    }

    if (blockToUse->size == size)
    {
        // use whole block
        removeFromFreeList(blockToUse);
        addToUsedList(blockToUse);
        return blockToUse->address;
    }
    else
    {
        // split block
        MemoryBlock* newBlock = new MemoryBlock(blockToUse->address, size);
        addToUsedList(newBlock);

        blockToUse->address += size;
        blockToUse->size -= size;

        return newBlock->address;
    }
}


void Memory::freeMemory(uint8_t* ptr)
{
    MemoryBlock* block = _usedList;
    MemoryBlock* found = nullptr;
    while (_usedList)
    {
        if (block->address == ptr)
        {
            found = block;
            break;
        }
        block = block->next;
    }
    if(!found)
    {
        println("freeMemory for not used pointer called");
        _platform.fatalError();
    }
    removeFromUsedList(block);
    addToFreeList(block);
}

void Memory::writeMemory(uint32_t relativeAddress, size_t size, uint8_t* data)
{
    memcpy(toAbsolute(relativeAddress), data, size);
}


uint8_t* Memory::toAbsolute(uint32_t relativeAddress)
{
    return _data + (ptrdiff_t)relativeAddress;
}


uint32_t Memory::toRelative(uint8_t* absoluteAddress)
{
    return absoluteAddress - _data;
}

MemoryBlock* Memory::removeFromList(MemoryBlock* head, MemoryBlock* item)
{
    if (head == item)
    {
        MemoryBlock* newHead = head->next;
        head->next = nullptr;
        return newHead;
    }

    if (!head || !item)
    {
        println("invalid parameters of Memory::removeFromList");
        _platform.fatalError();
    }

    bool found = false;
    while (head)
    {
        if (head->next == item)
        {
            found = true;
            head->next = item->next;
            break;
        }
        head = head->next;
    }

    if (!found)
    {
        println("tried to remove block from list not in it");
        _platform.fatalError();
    }
    item->next = nullptr;
    return head;
}

void Memory::removeFromFreeList(MemoryBlock* block)
{
    _freeList = removeFromList(_freeList, block);
}


void Memory::removeFromUsedList(MemoryBlock* block)
{
    _usedList = removeFromList(_usedList, block);
}


void Memory::addToUsedList(MemoryBlock* block)
{
    block->next = _usedList;
    _usedList = block;
}


void Memory::addToFreeList(MemoryBlock* block)
{
    if (_freeList == nullptr)
    {
        _freeList = block;
        return;
    }

    // first insert free block in list
    MemoryBlock* current = _freeList;
    while (current)
    {
        if (current->address <= block->address && (current->next == nullptr || block->address < current->next->address))
        {
            //add after current
            block->next = current->next;
            current->next = block;
            break;
        }
        else if (current->address > block->address)
        {
            //add before current
            block->next = current;

            if (current == _freeList)
                _freeList = block;

            // swap current and block for merge
            MemoryBlock* tmp = current;
            current = block;
            block = tmp;

            break;
        }

        current = current->next;
    }
    // now check if we can merge the blocks
    // first current an block
    if ((current->address + current->size) == block->address)
    {
        current->size += block->size;
        current->next = block->next;
        delete block;
        // check further if now current can be merged with current->next
        block = current;
    }

    // if block is the last one, we are done 
    if (block->next == nullptr)
        return;

    // now check block and block->next
    if ((block->address + block->size) == block->next->address)
    {
        block->size += block->next->size;
        block->next = block->next->next;
        delete block->next;
    }
}

uint16_t Memory::alignToPageSize(size_t size)
{
    // to 32 bit for now
    return (size + 3) & ~0x3;
}

MemoryBlock* Memory::findBlockInList(MemoryBlock* head, uint8_t* address)
{
    while (head != nullptr)
    {
        if (head->address == address)
            return head;

        head = head->next;
    }
    return nullptr;
}

void Memory::addNewUsedBlock(uint8_t* address, size_t size)
{
    MemoryBlock* smallerFreeBlock = _freeList;
    // find block in freeList where the new used block is contained in
    while (smallerFreeBlock)
    {
        if (smallerFreeBlock->next == nullptr ||
            (smallerFreeBlock->next != nullptr && smallerFreeBlock->next->address > address))
            break;
        
        smallerFreeBlock = smallerFreeBlock->next;
    }

    if (smallerFreeBlock == nullptr)
    {
        println("addNewUsedBlock: no smallerBlock found");
        _platform.fatalError();
    }

    if ((smallerFreeBlock->address + smallerFreeBlock->size) < (address + size))
    {
        println("addNewUsedBlock: found block can't contain new block");
        _platform.fatalError();
    }

    if (smallerFreeBlock->address == address && smallerFreeBlock->size == size)
    {
        // we take thow whole block
        removeFromFreeList(smallerFreeBlock);
        addToUsedList(smallerFreeBlock);
        return;
    }

    if (smallerFreeBlock->address == address)
    {
        // we take a front part of the block
        smallerFreeBlock->address += size;
        smallerFreeBlock->size -= size;
    }
    else
    {
        // we take a middle or end part of the block
        uint8_t* oldEndAddr = smallerFreeBlock->address + smallerFreeBlock->size;
        smallerFreeBlock->size -= (address - smallerFreeBlock->address);

        if (address + size < oldEndAddr)
        {
            // we take the middle part of the block, so we need a new free block for the end part
            MemoryBlock* newFreeBlock = new MemoryBlock();
            newFreeBlock->next = smallerFreeBlock->next;
            newFreeBlock->address = address + size;
            newFreeBlock->size = oldEndAddr - newFreeBlock->address;
            smallerFreeBlock->next = newFreeBlock;
        }
    }

    MemoryBlock* newUsedBlock = new MemoryBlock(address, size);
    addToUsedList(newUsedBlock);
}