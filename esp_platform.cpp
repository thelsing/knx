#include "esp_platform.h"
#include <user_interface.h>
#include <Arduino.h>
#include <EEPROM.h>

EspPlatform::EspPlatform()
{
}

uint32_t EspPlatform::currentIpAddress()
{
    return WiFi.localIP();
}

uint32_t EspPlatform::currentSubnetMask()
{
    return WiFi.subnetMask();
}

uint32_t EspPlatform::currentDefaultGateway()
{
    return WiFi.gatewayIP();
}

void EspPlatform::macAddress(uint8_t * addr)
{
    wifi_get_macaddr(STATION_IF, addr);
}

uint32_t EspPlatform::millis()
{
    return millis();
}

void EspPlatform::mdelay(uint32_t millis)
{
    delay(millis);
}

void EspPlatform::restart()
{
    ESP.restart();
}

void EspPlatform::fatalError()
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

void EspPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    _mulitcastAddr = addr;
    _mulitcastPort = port;
    _udp.beginMulticast(WiFi.localIP(), addr, port);
}

void EspPlatform::closeMultiCast()
{
    _udp.stop();
}

bool EspPlatform::sendBytes(uint8_t * buffer, uint16_t len)
{
    _udp.beginPacketMulticast(_mulitcastAddr, _mulitcastPort, WiFi.localIP());
    _udp.write(buffer, len);
    _udp.endPacket();
    return true;
}

int EspPlatform::readBytes(uint8_t * buffer, uint16_t maxLen)
{
    int len = _udp.parsePacket();
    if (len == 0)
        return 0;
    
    if (len > maxLen)
    {
        printf("udp buffer to small. was %d, needed %d\n", maxLen, len);
        fatalError();
    }

    _udp.read(buffer, len);
    return len;
}

uint8_t * EspPlatform::getEepromBuffer(uint16_t size)
{
    EEPROM.begin(size);
    return EEPROM.getDataPtr();
}

void EspPlatform::commitToEeprom()
{
    EEPROM.commit();
}
