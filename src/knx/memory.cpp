#include "memory.h"

#include <string.h>

#include "bits.h"

Memory::Memory(Platform& platform, DeviceObject& deviceObject)
    : _platform(platform), _deviceObject(deviceObject)
{}

Memory::~Memory()
{}

void Memory::readMemory()
{
    println("readMemory");

    uint8_t* flashStart = _platform.getNonVolatileMemoryStart();
    size_t flashSize = _platform.getNonVolatileMemorySize();
    if (flashStart == nullptr)
    {
        println("no user flash available;");
        return;
    }

    printHex("RESTORED ", flashStart, _metadataSize);

    uint16_t metadataBlockSize = alignToPageSize(_metadataSize);

    _freeList = new MemoryBlock(flashStart + metadataBlockSize, flashSize - metadataBlockSize);

    uint16_t apiVersion = 0;
    const uint8_t* buffer = popWord(apiVersion, flashStart);

    uint16_t manufacturerId = 0;
    buffer = popWord(manufacturerId, buffer);

    uint8_t hardwareType[LEN_HARDWARE_TYPE] = {0};
    buffer = popByteArray(hardwareType, LEN_HARDWARE_TYPE, buffer);

    uint16_t version = 0;
    buffer = popWord(version, buffer);

    VersionCheckResult versionCheck = FlashAllInvalid;

    // first check correct format of deviceObject-API
    if (_deviceObject.apiVersion == apiVersion) 
    {
        if (_versionCheckCallback != 0) {
            versionCheck = _versionCheckCallback(manufacturerId, hardwareType, version);
            // callback should provide infomation about version check failure reasons
        }
        else if (_deviceObject.manufacturerId() == manufacturerId &&
                 memcmp(_deviceObject.hardwareType(), hardwareType, LEN_HARDWARE_TYPE) == 0) 
        {
            if (_deviceObject.version() == version) {
                versionCheck = FlashValid;
            } 
            else
            {
                versionCheck = FlashTablesInvalid;
            }
        } 
        else 
        {
            println("manufacturerId or hardwareType are different");
            print("expexted manufacturerId: ");
            print(_deviceObject.manufacturerId(), HEX);
            print(", stored manufacturerId: ");
            println(manufacturerId, HEX);
            print("expexted hardwareType: ");
            printHex("", _deviceObject.hardwareType(), LEN_HARDWARE_TYPE);
            print(", stored hardwareType: ");
            printHex("", hardwareType, LEN_HARDWARE_TYPE);
            println("");
        }
    } 
    else 
    {
        println("DataObject api changed, any data stored in flash is invalid.");
        print("expexted DataObject api version: ");
        print(_deviceObject.apiVersion, HEX);
        print(", stored api version: ");
        println(apiVersion, HEX);
    }

    if (versionCheck == FlashAllInvalid)
    {
        println("ETS has to reprogram PA and application!");
        return;
    }

    println("restoring data from flash...");
    print("saverestores ");
    println(_saveCount);
    for (int i = 0; i < _saveCount; i++)
    {
        println(flashStart - buffer);
        println(".");
        buffer = _saveRestores[i]->restore(buffer);
    }
    println("restored saveRestores");
    if (versionCheck == FlashTablesInvalid) 
    {
        println("TableObjects are referring to an older firmware version and are not loaded");
        return;
    }
    print("tableObjs ");
    println(_tableObjCount);
    for (int i = 0; i < _tableObjCount; i++)
    {
        println(flashStart - buffer);
        println(".");
        buffer = _tableObjects[i]->restore(buffer);
        uint16_t memorySize = 0;
        buffer = popWord(memorySize, buffer);
        println(memorySize);
        if (memorySize == 0)
            continue;

        // this works because TableObject saves a relative addr and restores it itself
        addNewUsedBlock(_tableObjects[i]->_data, memorySize);
    }
    println("restored Tableobjects");
}

void Memory::writeMemory()
{
    // first get the necessary size of the writeBuffer
    uint16_t writeBufferSize = _metadataSize;
    for (int i = 0; i < _saveCount; i++)
        writeBufferSize = MAX(writeBufferSize, _saveRestores[i]->saveSize());

    for (int i = 0; i < _tableObjCount; i++)
        writeBufferSize = MAX(writeBufferSize, _tableObjects[i]->saveSize() + 2 /*for memory pos*/);
    
    uint8_t buffer[writeBufferSize];
    uint32_t flashPos = 0;
    uint8_t* bufferPos = buffer;

    bufferPos = pushWord(_deviceObject.apiVersion, bufferPos);
    bufferPos = pushWord(_deviceObject.manufacturerId(), bufferPos);
    bufferPos = pushByteArray(_deviceObject.hardwareType(), LEN_HARDWARE_TYPE, bufferPos);
    bufferPos = pushWord(_deviceObject.version(), bufferPos);

    flashPos = _platform.writeNonVolatileMemory(flashPos, buffer, bufferPos - buffer);

    print("save saveRestores ");
    println(_saveCount);
    for (int i = 0; i < _saveCount; i++)
    {
        bufferPos = _saveRestores[i]->save(buffer);
        flashPos = _platform.writeNonVolatileMemory(flashPos, buffer, bufferPos - buffer);
    }

    print("save tableobjs ");
    println(_tableObjCount);
    for (int i = 0; i < _tableObjCount; i++)
    {
        bufferPos = _tableObjects[i]->save(buffer);

        //save to size of the memoryblock for tableobject too, so that we can rebuild the usedList and freeList
        if (_tableObjects[i]->_data != nullptr)
        {
            MemoryBlock* block = findBlockInList(_usedList, _tableObjects[i]->_data);
            if (block == nullptr)
            {
                println("_data of TableObject not in _usedList");
                _platform.fatalError();
            }
            bufferPos = pushWord(block->size, bufferPos);
        }
        else
            bufferPos = pushWord(0, bufferPos);

        flashPos = _platform.writeNonVolatileMemory(flashPos, buffer, bufferPos - buffer);
    }
    
    _platform.commitNonVolatileMemory();
}

void Memory::saveMemory()
{
    _platform.commitNonVolatileMemory();
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
    // always allocate aligned to pagesize
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
    while (block)
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
    _platform.writeNonVolatileMemory(relativeAddress, data, size);
}

void Memory::readMemory(uint32_t relativeAddress, size_t size, uint8_t* data)
{
    _platform.readNonVolatileMemory(relativeAddress, data, size);
}


uint8_t* Memory::toAbsolute(uint32_t relativeAddress)
{
    return _platform.getNonVolatileMemoryStart() + (ptrdiff_t)relativeAddress;
}


uint32_t Memory::toRelative(uint8_t* absoluteAddress)
{
    return absoluteAddress - _platform.getNonVolatileMemoryStart();
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
    MemoryBlock* block = head;
    while (block)
    {
        if (block->next == item)
        {
            found = true;
            block->next = item->next;
            break;
        }
        block = block->next;
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
    size_t pageSize = 4; //_platform.flashPageSize(); // align to 32bit for now, as aligning to flash-page-size causes side effects in programming
    // pagesize should be a multiply of two
    return (size + pageSize - 1) & (-1*pageSize);
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
        smallerFreeBlock->size = (address - smallerFreeBlock->address);

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

void Memory::versionCheckCallback(VersionCheckCallback func)
{
    _versionCheckCallback = func;
}

VersionCheckCallback Memory::versionCheckCallback()
{
    return _versionCheckCallback;
}
