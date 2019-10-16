#pragma once

#include <stdint.h>
#include <stddef.h>
#include "save_restore.h"

class Platform
{
  public:
    Platform();
    virtual uint32_t currentIpAddress() = 0;
    virtual uint32_t currentSubnetMask() = 0;
    virtual uint32_t currentDefaultGateway() = 0;
    virtual void macAddress(uint8_t* data) = 0;

    virtual void restart() = 0;
    virtual void fatalError() = 0;

    virtual void setupMultiCast(uint32_t addr, uint16_t port) = 0;
    virtual void closeMultiCast() = 0;
    virtual bool sendBytes(uint8_t* buffer, uint16_t len) = 0;
    virtual int readBytes(uint8_t* buffer, uint16_t maxLen) = 0;

    virtual void setupUart() = 0;
    virtual void closeUart() = 0;
    virtual int uartAvailable() = 0;
    virtual size_t writeUart(const uint8_t data) = 0;
    virtual size_t writeUart(const uint8_t* buffer, size_t size) = 0;
    virtual int readUart() = 0;
    virtual size_t readBytesUart(uint8_t* buffer, size_t length) = 0;

    /**
     * Provides a memory block for an interface object.
     * Hereby, the type of memory is not important, as long as at the latest with finishNVMemory(), the data is written to a non-volatile memory.
     * Is called by the stack every time ETS wants to store data (device, application, association, ....)
     * The data itself is then written by writeNVMemory()
     *
     * @param size The number of bytes to allocate
     *
     * @param ID Unique identifier that represents the new memory block, even after reset
     *
     * @return  The start address of the created memory block
     */
    virtual uint8_t* allocNVMemory(size_t size,uint32_t ID) = 0;

    /**
     * Write one single byte to previously allocated (allocNVMemory()) or reloaded (reloadNVMemory()) memory block
     * Is called by the stack during ETS programming to store interface object data and before reset (save()) to store object status
     *
     * @param addr The address where to store the data
     *
     * @param data The value of the data which sould be stored
     *
     */
    virtual bool writeNVMemory(uint8_t* addr,uint8_t data) = 0;

    /**
     * Reads one single byte from previously allocated (allocNVMemory()) or reloaded (reloadNVMemory()) memory block
     * Is called by the stack during power up (restore()) and every time the application reads ETS parameter values (knx.paramByte())
     *
     * @param addr The address where to read from
     *
     * @return The value of the data byte which was read
     *
     */
    virtual uint8_t readNVMemory(uint8_t* addr) = 0;

    /**
     * Returns existing memory block identified by the ID, even after reset
     * Is called by the stack during power up (restore())
     *
     * @param ID Unique identifier that represents a previously allocated memory block
     *
     * @param pointerAccess TRUE if the memory block must be accessible through pointer dereferencing, otherwise FALSE
     *
     * @return The start address of the memory block represented by ID or NULL if ID is not available
     *
     */
    virtual uint8_t* reloadNVMemory(uint32_t ID, bool pointerAccess) = 0;

    /**
     * Is called by the stack after every ETS programming cycle (save())
     * This function may be used to write ram buffered memory block to a non-volatile one
     *
     */
    virtual void finishNVMemory() = 0;

    /**
     * Frees existing memory block identified by the ID
     * Is called by the stack every time ETS wants to store new data and before it allocates new one
     *
     * @param ID Unique identifier that represents a previously allocated memory block
     *
     */
    virtual void freeNVMemory(uint32_t ID) = 0;

    /**
     * Provides the offset of the NVMemory address to the ETS address space (16bit)
     * highest NVMemory address - referenceNVMemory() <= 16bit
     *
     * @return The offset address if NVMemory address space is >16bit otherwise NULL
     *
     */
    virtual uint8_t* referenceNVMemory();


    virtual uint8_t* allocMemory(size_t size);
    virtual void freeMemory(uint8_t* ptr);

    uint8_t popNVMemoryByte(uint8_t** addr);
    uint16_t popNVMemoryWord(uint8_t** addr);
    uint32_t popNVMemoryInt(uint8_t** addr);
    void popNVMemoryArray(uint8_t* dest, uint8_t** addr, size_t size);
    void pushNVMemoryByte(uint8_t val, uint8_t** addr);
    void pushNVMemoryWord(uint16_t val, uint8_t** addr);
    void pushNVMemoryInt(uint32_t val, uint8_t** addr);
    void pushNVMemoryArray(uint8_t* src, uint8_t** addr, size_t size);
  protected:
    uint8_t* _memoryReference = 0;
};
