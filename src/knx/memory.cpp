#include "memory.h"
#include <string.h>
#include "bits.h"

Memory::Memory(Platform& platform, DeviceObject& deviceObject)
    : _platform(platform), _deviceObject(deviceObject)
{
}

void Memory::memoryModified()
{
    _modified = true;
}

bool Memory::isMemoryModified()
{
    return _modified;
}

// TODO implement flash layout: manufacturerID, HarwareType, Version, addr[0], size[0], addr[1], size[1], ...
// reconstruct free flash list and used list on read
void Memory::readMemory()
{
    _data = _platform.getEepromBuffer(512);

    if (_data[0] != 0x00 || _data[1] != 0xAD || _data[2] != 0xAF || _data[3] != 0xFE)
        return;

    uint8_t* buffer = _data + 4;
    int size = _saveCount;
    for (int i = 0; i < size; i++)
    {
        buffer = _saveRestores[i]->restore(buffer);
    }
}

void Memory::writeMemory()
{
    uint8_t* buffer = _data;
    buffer = pushWord(_deviceObject.manufacturerId(), buffer);
    buffer = pushByteArray(_deviceObject.hardwareType(), 6, buffer);
    buffer = pushWord(_deviceObject.version(), buffer);

    int size = _saveCount;
    for (int i = 0; i < size; i++)
    {
        // TODO: Hande TableObject correctly, i.e. save the size of the buffer somewhere to, so that we can rebuild the usedList and freeList
        buffer = _saveRestores[i]->save(buffer);
    }
    _platform.commitToEeprom();
    _modified = false;
}

void Memory::addSaveRestore(SaveRestore * obj)
{
    if (_saveCount >= MAXSAVE - 1)
        return;

    _saveRestores[_saveCount] = obj;
    _saveCount += 1;
}


uint8_t* Memory::allocMemory(size_t size)
{
    // always allocate aligned to 32 bit
    size = (size + 3) & ~0x3;
    
    MemoryBlock* freeBlock = _freeList;
    MemoryBlock* blockToUse = 0;
    
    // find the smallest possible block that is big enough
    while (freeBlock)
    {
        if (freeBlock->size >= size)
        {
            if (blockToUse != 0 && (blockToUse->size - size) > (freeBlock->size - size))
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
        MemoryBlock* newBlock = new MemoryBlock();
        newBlock->address = blockToUse->address;
        newBlock->size = size;
        addToUsedList(newBlock);

        blockToUse->address += size;
        blockToUse->size -= size;

        return newBlock->address;
    }
}


void Memory::freeMemory(uint8_t* ptr)
{
    MemoryBlock* block = _usedList;
    MemoryBlock* found = 0;
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
    memoryModified();
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
        head->next = 0;
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
    item->next = 0;
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
    if (_freeList == 0)
    {
        _freeList = block;
        return;
    }

    // first insert free block in list
    MemoryBlock* current = _freeList;
    while (current)
    {
        if (current->address <= block->address && (block->next == 0 || block->address < current->next->address))
        {
            block->next = current->next;
            current->next = block;
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
    if (block->next == 0)
        return;

    // now check block and block->next
    if ((block->address + block->size) == block->next->address)
    {
        block->size += block->next->size;
        block->next = block->next->next;
        delete block->next;
    }
}
