#ifdef ARDUINO_ARCH_ESP32
#include "arduino_platform.h"
#include <WiFi.h>
#include <WiFiUdp.h>

#define MAX_MEMORY_BLOCKS   6

typedef struct{
    uint32_t ID;
    size_t  size;
    uint8_t* data;
}MemoryBlock_t;

class Esp32Platform : public ArduinoPlatform
{
    using ArduinoPlatform::_mulitcastAddr;
    using ArduinoPlatform::_mulitcastPort;
public:
    Esp32Platform();
    Esp32Platform( HardwareSerial* s);

    // ip stuff
    uint32_t currentIpAddress() override;
    uint32_t currentSubnetMask() override;
    uint32_t currentDefaultGateway() override;
    void macAddress(uint8_t* addr) override;

    // basic stuff
    void restart();

    //multicast
    void setupMultiCast(uint32_t addr, uint16_t port) override;
    void closeMultiCast() override;
    bool sendBytes(uint8_t* buffer, uint16_t len) override;
    int readBytes(uint8_t* buffer, uint16_t maxLen) override;
    
    //memory
    bool writeNVMemory(uint8_t* addr,uint8_t data);
    uint8_t readNVMemory(uint8_t* addr);
    uint8_t* allocNVMemory(size_t size,uint32_t ID);
    uint8_t* reloadNVMemory(uint32_t ID, bool pointerAccess);
    void finishNVMemory();
    void freeNVMemory(uint32_t ID);
private:
    WiFiUDP _udp;
    void initNVMemory();
    MemoryBlock_t _memoryBlocks[MAX_MEMORY_BLOCKS];
    bool _MemoryInitialized = false;
};

#endif
