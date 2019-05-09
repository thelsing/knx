#include "esp_platform.h"

#ifdef ARDUINO_ARCH_ESP8266
#include <user_interface.h>
#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

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
    return ::millis();
}

void EspPlatform::mdelay(uint32_t millis)
{
    delay(millis);
}

void EspPlatform::restart()
{
    Serial.println("restart");
    ESP.reset();
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
    _mulitcastAddr = htonl(addr);
    _mulitcastPort = port;
    IPAddress mcastaddr(_mulitcastAddr);
    
    Serial.printf("setup multicast addr: %s port: %d ip: %s\n", mcastaddr.toString().c_str(), port,
        WiFi.localIP().toString().c_str());
    uint8 result = _udp.beginMulticast(WiFi.localIP(), mcastaddr, port);
    Serial.printf("result %d\n", result);
}

void EspPlatform::closeMultiCast()
{
    _udp.stop();
}

bool EspPlatform::sendBytes(uint8_t * buffer, uint16_t len)
{
    //printHex("<- ",buffer, len);
    int result = 0;
    result = _udp.beginPacketMulticast(_mulitcastAddr, _mulitcastPort, WiFi.localIP());
    result = _udp.write(buffer, len);
    result = _udp.endPacket();
    return true;
}

int EspPlatform::readBytes(uint8_t * buffer, uint16_t maxLen)
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

uint8_t * EspPlatform::getEepromBuffer(uint16_t size)
{
    EEPROM.begin(size);
    return EEPROM.getDataPtr();
}

void EspPlatform::commitToEeprom()
{
    EEPROM.commit();
}

void EspPlatform::setupUart()
{
    Serial.begin(19200, SERIAL_8E1);
    while (!Serial) ;
}


void EspPlatform::closeUart()
{
    Serial.end();
}


int EspPlatform::uartAvailable()
{
    return Serial.available();
}


size_t EspPlatform::writeUart(const uint8_t data)
{
    printHex("<p", &data, 1);
    return Serial.write(data);
}


size_t EspPlatform::writeUart(const uint8_t *buffer, size_t size)
{
    printHex("<p", buffer, size);
    return Serial.write(buffer, size);
}


int EspPlatform::readUart()
{
    int val = Serial.read();
    if (val > 0)
        printHex("p>", (uint8_t*)&val, 1);
    return val;
}


size_t EspPlatform::readBytesUart(uint8_t *buffer, size_t length)
{
    size_t toRead = length;
    uint8_t* pos = buffer;
    while (toRead > 0)
    {
        size_t val = Serial.readBytes(pos, toRead);
        pos += val;
        toRead -= val;
    }
    
    
    printHex("p>", buffer, length);
    return length;
}

void print(const char* s)
{
    printf("%s", s);
}
void print(char c)
{
    printf("%c", c);
}

void print(unsigned char num)
{
    print(num, DEC);
}

void print(unsigned char num, int base)
{
    if (base == HEX)
        printf("%X", num);
    else
        printf("%d", num);
}

void print(int num)
{
    print(num, DEC);
}

void print(int num, int base)
{
    if (base == HEX)
        printf("%X", num);
    else
        printf("%d", num);
}

void print(unsigned int num)
{
    print(num, DEC);
}

void print(unsigned int num, int base)
{
    if (base == HEX)
        printf("%X", num);
    else
        printf("%d", num);
}

void print(long num)
{
    print(num, DEC);
}

void print(long num, int base)
{
    if (base == HEX)
        printf("%lX", num);
    else
        printf("%ld", num);
}

void print(unsigned long num)
{
    print(num, DEC);
}

void print(unsigned long num, int base)
{
    if (base == HEX)
        printf("%lX", num);
    else
        printf("%ld", num);
}

void print(double num)
{
    printf("%f", num);
}

void println(const char* s)
{
    printf("%s\n", s);
}
void println(char c)
{
    printf("%c\n", c);
}

void println(unsigned char num)
{
    println(num, DEC);
}

void println(unsigned char num, int base)
{
    if (base == HEX)
        printf("%X\n", num);
    else
        printf("%d\n", num);
}

void println(int num)
{
    println(num, DEC);
}

void println(int num, int base)
{
    if (base == HEX)
        printf("%X\n", num);
    else
        printf("%d\n", num);
}

void println(unsigned int num)
{
    println(num, DEC);
}

void println(unsigned int num, int base)
{
    if (base == HEX)
        printf("%X\n", num);
    else
        printf("%d\n", num);
}

void println(long num)
{
    println(num, DEC);
}

void println(long num, int base)
{
    if (base == HEX)
        printf("%lX\n", num);
    else
        printf("%ld\n", num);
}

void println(unsigned long num)
{
    println(num, DEC);
}

void println(unsigned long num, int base)
{
    if (base == HEX)
        printf("%lX\n", num);
    else
        printf("%ld\n", num);
}

void println(double num)
{
    printf("%f\n", num);
}

void println(void)
{
    printf("\n");
}
#endif
