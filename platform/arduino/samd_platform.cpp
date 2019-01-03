#include "samd_platform.h"

#ifdef ARDUINO_ARCH_SAMD
#include <knx/bits.h>

#include <Arduino.h>
#include <FlashAsEEPROM.h>

SamdPlatform::SamdPlatform()
{
}

uint32_t SamdPlatform::currentIpAddress()
{
    // not needed
    return 0;
}

uint32_t SamdPlatform::currentSubnetMask()
{
    // not needed
    return 0;
}

uint32_t SamdPlatform::currentDefaultGateway()
{
    // not needed
    return 0;
}

void SamdPlatform::macAddress(uint8_t * addr)
{
    // not needed
}

uint32_t SamdPlatform::millis()
{
    return::millis();
}

void SamdPlatform::mdelay(uint32_t millis)
{
    delay(millis);
}

void SamdPlatform::restart()
{
    SerialUSB.println("restart");
    NVIC_SystemReset();
}

void SamdPlatform::fatalError()
{
    const int period = 200;
    while (true)
    {
        if ((millis() % period) > (period / 2))
            digitalWrite(LED_BUILTIN, HIGH);
        else
            digitalWrite(LED_BUILTIN, LOW);
    }
}

void SamdPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    //not needed
}

void SamdPlatform::closeMultiCast()
{
    //not needed
}

bool SamdPlatform::sendBytes(uint8_t * buffer, uint16_t len)
{
    //not needed
}

int SamdPlatform::readBytes(uint8_t * buffer, uint16_t maxLen)
{
    //not needed
    return 0;
}

uint8_t * SamdPlatform::getEepromBuffer(uint16_t size)
{
    //EEPROM.begin(size);
    if(size > EEPROM_EMULATION_SIZE)
        fatalError();
    
    return EEPROM.getDataPtr();
}

void SamdPlatform::commitToEeprom()
{
    EEPROM.commit();
}


void SamdPlatform::setupUart()
{
    SerialKNX.begin(19200, SERIAL_8E1);
    while (!SerialKNX) 
        ;
}


void SamdPlatform::closeUart()
{
    SerialKNX.end();
}


int SamdPlatform::uartAvailable()
{
    return SerialKNX.available();
}


size_t SamdPlatform::writeUart(const uint8_t data)
{
    //printHex("<p", &data, 1);
    return SerialKNX.write(data);
}


size_t SamdPlatform::writeUart(const uint8_t *buffer, size_t size)
{
    //printHex("<p", buffer, size);
    return SerialKNX.write(buffer, size);
}


int SamdPlatform::readUart()
{
    int val = SerialKNX.read();
    //if(val > 0)
    //    printHex("p>", (uint8_t*)&val, 1);
    return val;
}


size_t SamdPlatform::readBytesUart(uint8_t *buffer, size_t length)
{
    size_t toRead = length;
    uint8_t* pos = buffer;
    while (toRead > 0)
    {
        size_t val = SerialKNX.readBytes(pos, toRead);
        pos += val;
        toRead -= val;
    }
    //printHex("p>", buffer, length);
    return length;
}
#endif