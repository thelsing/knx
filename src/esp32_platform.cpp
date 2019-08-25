#include "esp32_platform.h"

#ifdef ARDUINO_ARCH_ESP32
#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

#define SerialDBG Serial

Esp32Platform::Esp32Platform()
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
    Serial.println("restart");
    ESP.restart();
}

void Esp32Platform::fatalError()
{
    Serial.println("GURU MEDITATION - fatal error!");
    const int period = 200;
    while (true)
    {
#ifdef LED_BUILTIN
        if ((millis() % period) > (period / 2))
            digitalWrite(LED_BUILTIN, HIGH);
        else
            digitalWrite(LED_BUILTIN, LOW);
#endif
    }
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

uint8_t * Esp32Platform::getEepromBuffer(uint16_t size)
{
    EEPROM.begin(size);
    return EEPROM.getDataPtr();
}

void Esp32Platform::commitToEeprom()
{
    EEPROM.commit();
}

void Esp32Platform::setupUart()
{
    Serial.begin(19200, SERIAL_8E1);
    while (!Serial) ;
}


void Esp32Platform::closeUart()
{
    Serial.end();
}


int Esp32Platform::uartAvailable()
{
    return Serial.available();
}


size_t Esp32Platform::writeUart(const uint8_t data)
{
    printHex("<p", &data, 1);
    return Serial.write(data);
}


size_t Esp32Platform::writeUart(const uint8_t *buffer, size_t size)
{
    printHex("<p", buffer, size);
    return Serial.write(buffer, size);
}


int Esp32Platform::readUart()
{
    int val = Serial.read();
    if (val > 0)
        printHex("p>", (uint8_t*)&val, 1);
    return val;
}


size_t Esp32Platform::readBytesUart(uint8_t *buffer, size_t length)
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
    Serial.printf("%s", s);
}
void print(char c)
{
    Serial.printf("%c", c);
}

void print(unsigned char num)
{
    print(num, DEC);
}

void print(unsigned char num, int base)
{
    if (base == HEX)
        Serial.printf("%X", num);
    else
        Serial.printf("%d", num);
}

void print(int num)
{
    print(num, DEC);
}

void print(int num, int base)
{
    if (base == HEX)
        Serial.printf("%X", num);
    else
        Serial.printf("%d", num);
}

void print(unsigned int num)
{
    print(num, DEC);
}

void print(unsigned int num, int base)
{
    if (base == HEX)
        Serial.printf("%X", num);
    else
        Serial.printf("%d", num);
}

void print(long num)
{
    print(num, DEC);
}

void print(long num, int base)
{
    if (base == HEX)
        Serial.printf("%lX", num);
    else
        Serial.printf("%ld", num);
}

void print(unsigned long num)
{
    print(num, DEC);
}

void print(unsigned long num, int base)
{
    if (base == HEX)
        Serial.printf("%lX", num);
    else
        Serial.printf("%ld", num);
}

void print(double num)
{
    Serial.printf("%f", num);
}

void println(const char* s)
{
    Serial.printf("%s\n", s);
}
void println(char c)
{
    Serial.printf("%c\n", c);
}

void println(unsigned char num)
{
    println(num, DEC);
}

void println(unsigned char num, int base)
{
    if (base == HEX)
        Serial.printf("%X\n", num);
    else
        Serial.printf("%d\n", num);
}

void println(int num)
{
    println(num, DEC);
}

void println(int num, int base)
{
    if (base == HEX)
        Serial.printf("%X\n", num);
    else
        Serial.printf("%d\n", num);
}

void println(unsigned int num)
{
    println(num, DEC);
}

void println(unsigned int num, int base)
{
    if (base == HEX)
        Serial.printf("%X\n", num);
    else
        Serial.printf("%d\n", num);
}

void println(long num)
{
    println(num, DEC);
}

void println(long num, int base)
{
    if (base == HEX)
        Serial.printf("%lX\n", num);
    else
        Serial.printf("%ld\n", num);
}

void println(unsigned long num)
{
    println(num, DEC);
}

void println(unsigned long num, int base)
{
    if (base == HEX)
        Serial.printf("%lX\n", num);
    else
        Serial.printf("%ld\n", num);
}

void println(double num)
{
    Serial.printf("%f\n", num);
}

void println(void)
{
    Serial.printf("\n");
}
#endif
