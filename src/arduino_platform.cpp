#include "arduino_platform.h"
#include "knx/bits.h"

#include <Arduino.h>
#include <SPI.h>

Stream* ArduinoPlatform::SerialDebug = &Serial;

ArduinoPlatform::ArduinoPlatform(HardwareSerial* knxSerial) : _knxSerial(knxSerial)
{
}

void ArduinoPlatform::fatalError()
{
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

void ArduinoPlatform::knxUart( HardwareSerial* serial )
{
    closeUart();
    _knxSerial = serial;
    setupUart();
}

HardwareSerial* ArduinoPlatform::knxUart()
{
    return _knxSerial;
}

void ArduinoPlatform::setupUart()
{
    _knxSerial->begin(19200, SERIAL_8E1);
    while (!_knxSerial) 
        ;
}


void ArduinoPlatform::closeUart()
{
    _knxSerial->end();
}


int ArduinoPlatform::uartAvailable()
{
    return _knxSerial->available();
}


size_t ArduinoPlatform::writeUart(const uint8_t data)
{
    //printHex("<p", &data, 1);
    return _knxSerial->write(data);
}


size_t ArduinoPlatform::writeUart(const uint8_t *buffer, size_t size)
{
    //printHex("<p", buffer, size);
    return _knxSerial->write(buffer, size);
}


int ArduinoPlatform::readUart()
{
    int val = _knxSerial->read();
    //if(val > 0)
    //    printHex("p>", (uint8_t*)&val, 1);
    return val;
}


size_t ArduinoPlatform::readBytesUart(uint8_t *buffer, size_t length)
{
    size_t toRead = length;
    uint8_t* pos = buffer;
    while (toRead > 0)
    {
        size_t val = _knxSerial->readBytes(pos, toRead);
        pos += val;
        toRead -= val;
    }
    //printHex("p>", buffer, length);
    return length;
}

void ArduinoPlatform::setupSpi()
{
    SPI.begin();
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
}

void ArduinoPlatform::closeSpi()
{
    SPI.endTransaction();
    SPI.end();
}

int ArduinoPlatform::readWriteSpi(uint8_t *data, size_t len)
{
    SPI.transfer(data, len);
    return 0;
}

void printUint64(uint64_t value, int base = DEC)
  {
    char buf[8 * sizeof(uint64_t) + 1];
    char* str = &buf[sizeof(buf) - 1];
    *str = '\0';

    uint64_t n = value;
    do {
      char c = n % base;
      n /= base;

      *--str = c < 10 ? c + '0' : c + 'A' - 10;
    } while (n > 0);

     print(str);
}

void print(const char* s)
{
    ArduinoPlatform::SerialDebug->print(s);
}
void print(char c)
{
    ArduinoPlatform::SerialDebug->print(c);
}

void print(unsigned char num)
{
    ArduinoPlatform::SerialDebug->print(num);
}

void print(unsigned char num, int base)
{
    ArduinoPlatform::SerialDebug->print(num, base);
}

void print(int num)
{
    ArduinoPlatform::SerialDebug->print(num);
}

void print(int num, int base)
{
    ArduinoPlatform::SerialDebug->print(num, base);
}

void print(unsigned int num)
{
    ArduinoPlatform::SerialDebug->print(num);
}

void print(unsigned int num, int base)
{
    ArduinoPlatform::SerialDebug->print(num, base);
}

void print(long num)
{
    ArduinoPlatform::SerialDebug->print(num);
}

void print(long num, int base)
{
    ArduinoPlatform::SerialDebug->print(num, base);
}

void print(unsigned long num)
{
    ArduinoPlatform::SerialDebug->print(num);
}

void print(unsigned long num, int base)
{
    ArduinoPlatform::SerialDebug->print(num, base);
}

void print(unsigned long long num)
{
    printUint64(num);
}

void print(unsigned long long num, int base)
{
    printUint64(num, base);
}

void print(double num)
{
    ArduinoPlatform::SerialDebug->print(num);
}

void println(const char* s)
{
    ArduinoPlatform::SerialDebug->println(s);
}

void println(char c)
{
    ArduinoPlatform::SerialDebug->println(c);
}

void println(unsigned char num)
{
    ArduinoPlatform::SerialDebug->println(num);
}

void println(unsigned char num, int base)
{
    ArduinoPlatform::SerialDebug->println(num, base);
}

void println(int num)
{
    ArduinoPlatform::SerialDebug->println(num);
}

void println(int num, int base)
{
    ArduinoPlatform::SerialDebug->println(num, base);
}

void println(unsigned int num)
{
    ArduinoPlatform::SerialDebug->println(num);
}

void println(unsigned int num, int base)
{
    ArduinoPlatform::SerialDebug->println(num, base);
}

void println(long num)
{
    ArduinoPlatform::SerialDebug->println(num);
}

void println(long num, int base)
{
    ArduinoPlatform::SerialDebug->println(num, base);
}

void println(unsigned long num)
{
    ArduinoPlatform::SerialDebug->println(num);
}

void println(unsigned long num, int base)
{
    ArduinoPlatform::SerialDebug->println(num, base);
}

void println(unsigned long long num)
{
    printUint64(num);
    println("");
}

void println(unsigned long long num, int base)
{
    printUint64(num, base);
    println("");
}

void println(double num)
{
    ArduinoPlatform::SerialDebug->println(num);
}

void println(void)
{
    ArduinoPlatform::SerialDebug->println();
}
