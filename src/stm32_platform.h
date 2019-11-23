#include "knx/platform.h"

#define LED_BUILTIN 1

class Stm32Platform : public Platform
{
    bool m_initialized = false;
public:
    Stm32Platform();
    virtual ~Stm32Platform();

    virtual uint32_t currentIpAddress() override;
    virtual uint32_t currentSubnetMask() override;
    virtual uint32_t currentDefaultGateway() override;
    virtual void macAddress(uint8_t* data) override;

    virtual void restart() override;
    virtual void fatalError() override;

    virtual void setupMultiCast(uint32_t addr, uint16_t port) override;
    virtual void closeMultiCast() override;
    virtual bool sendBytes(uint8_t* buffer, uint16_t len) override;
    virtual int readBytes(uint8_t* buffer, uint16_t maxLen) override;

    virtual void setupUart() override;
    virtual void closeUart() override;
    virtual int uartAvailable() override;
    virtual size_t writeUart(const uint8_t data) override;
    virtual size_t writeUart(const uint8_t* buffer, size_t size) override;
    virtual int readUart() override;
    virtual size_t readBytesUart(uint8_t* buffer, size_t length) override;

    virtual uint8_t* getEepromBuffer(uint16_t size) override;
    virtual void commitToEeprom() override;

    //spi
    void setupSpi() override;
    void closeSpi() override;
    int readWriteSpi (uint8_t *data, size_t len) override;

};
