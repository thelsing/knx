#include "esp32_platform.h"

#ifdef ARDUINO_ARCH_ESP32
#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

Esp32Platform::Esp32Platform() : ArduinoPlatform(&Serial1)
{
}

Esp32Platform::Esp32Platform( HardwareSerial* s) : ArduinoPlatform(s)
{
}

uint32_t Esp32Platform::currentIpAddress()
{
    return WiFi.localIP();
}

uint32_t Esp32Platform::currentSubnetMask()
{
    return WiFi.subnetMask();
}

uint32_t Esp32Platform::currentDefaultGateway()
{
    return WiFi.gatewayIP();
}

void Esp32Platform::macAddress(uint8_t * addr)
{
    esp_wifi_get_mac(WIFI_IF_STA, addr);
}

void Esp32Platform::restart()
{
    println("restart");
    ESP.restart();
}

void Esp32Platform::setupMultiCast(uint32_t addr, uint16_t port)
{
    _mulitcastAddr = htonl(addr);
    _mulitcastPort = port;
    IPAddress mcastaddr(_mulitcastAddr);
    
    Serial.printf("setup multicast addr: %s port: %d ip: %s\n", mcastaddr.toString().c_str(), port,
        WiFi.localIP().toString().c_str());
    uint8_t result = _udp.beginMulticast(mcastaddr, port);
    Serial.printf("result %d\n", result);
}

void Esp32Platform::closeMultiCast()
{
    _udp.stop();
}

bool Esp32Platform::sendBytes(uint8_t * buffer, uint16_t len)
{
    //printHex("<- ",buffer, len);
    int result = 0;
    result = _udp.beginMulticastPacket();
    result = _udp.write(buffer, len);
    result = _udp.endPacket();
    return true;
}

int Esp32Platform::readBytes(uint8_t * buffer, uint16_t maxLen)
{
    int len = _udp.parsePacket();
    if (len == 0)
        return 0;
    
    if (len > maxLen)
    {
        Serial.printf("udp buffer to small. was %d, needed %d\n", maxLen, len);
        fatalError();
    }

    _udp.read(buffer, len);
    //printHex("-> ", buffer, len);
    return len;
}

bool Esp32Platform::writeNVMemory(uint8_t* addr,uint8_t data)
{
    *addr = data;
    return true;
}

uint8_t Esp32Platform::readNVMemory(uint8_t* addr)
{
    return *addr;
}

uint8_t* Esp32Platform::allocNVMemory(size_t size,uint32_t ID)
{
    int i;
    for(i=0;i<MAX_MEMORY_BLOCKS;i++){
        if(_memoryBlocks[i].ID == 0)
            break;
    }
    if(i >= MAX_MEMORY_BLOCKS)
        fatalError();


    _memoryBlocks[i].data = (uint8_t*)malloc(size);
    if(_memoryBlocks[i].data == NULL)
        fatalError();

    _memoryBlocks[i].ID = ID;
    _memoryBlocks[i].size = size;

    return _memoryBlocks[i].data;
}

void Esp32Platform::initNVMemory()
{
    EEPROM.begin(1024);
    uint32_t addr = 0;
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++){

        if (EEPROM.read(addr++) != 0xBA || EEPROM.read(addr++) != 0xAD || EEPROM.read(addr++) != 0xC0 || EEPROM.read(addr++) != 0xDE){
            _memoryBlocks[i].ID = 0;
            _memoryBlocks[i].size = 0;
            _memoryBlocks[i].data = NULL;
            continue;
        }

        ((uint8_t*)&_memoryBlocks[i].ID)[0] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].ID)[1] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].ID)[2] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].ID)[3] = EEPROM.read(addr++);

        ((uint8_t*)&_memoryBlocks[i].size)[0] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].size)[1] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].size)[2] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].size)[3] = EEPROM.read(addr++);

        _memoryBlocks[i].data = EEPROM.getDataPtr() + addr;
        addr += _memoryBlocks[i].size;
    }
}

uint8_t* Esp32Platform::reloadNVMemory(uint32_t ID, bool pointerAccess)
{
   if(!_MemoryInitialized)
       initNVMemory();

   _MemoryInitialized=true;

   int i;
   for(i=0;i<MAX_MEMORY_BLOCKS;i++){
       if(_memoryBlocks[i].ID == ID)
           break;
   }
   if(i >= MAX_MEMORY_BLOCKS)
       return 0;


   return _memoryBlocks[i].data;
}

void Esp32Platform::finishNVMemory()
{
    uint32_t addr = 0;

    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++)
    {
        if(_memoryBlocks[i].ID == 0)
            continue;

        //write valid mask
        EEPROM.write(addr++,0xBA);
        EEPROM.write(addr++,0xAD);
        EEPROM.write(addr++,0xC0);
        EEPROM.write(addr++,0xDE);

        //write ID
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[0]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[1]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[2]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[3]);

        //write size
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[0]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[1]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[2]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[3]);

        //write data
        for (uint32_t e=0;e<_memoryBlocks[i].size;e++){
            EEPROM.write(addr++,_memoryBlocks[i].data[e]);
        }
    }
    EEPROM.commit();
}

void Esp32Platform::freeNVMemory(uint32_t ID)
{
    int i;
    for(i=0;i<MAX_MEMORY_BLOCKS;i++){
        if(_memoryBlocks[i].ID == ID)
            break;
    }
    if(i >= MAX_MEMORY_BLOCKS)
        return;

    _memoryBlocks[i].data = NULL;
    _memoryBlocks[i].size = 0;
    _memoryBlocks[i].ID = 0;
}
#endif
