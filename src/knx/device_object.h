#pragma once

#include "interface_object.h"

#define LEN_HARDWARE_TYPE 6

class DeviceObject: public InterfaceObject
{
public:
    // increase this version anytime DeviceObject-API changes 
    // the following value represents the serialized representation of DeviceObject.
    const uint16_t apiVersion = 1;
    
    DeviceObject();
    uint8_t* save(uint8_t* buffer) override;
    const uint8_t* restore(const uint8_t* buffer) override;
    uint16_t saveSize() override;

    uint16_t individualAddress();
    void individualAddress(uint16_t value);

    void individualAddressDuplication(bool value);
    bool verifyMode();
    void verifyMode(bool value);
    bool progMode();
    void progMode(bool value);
    uint16_t manufacturerId();
    void manufacturerId(uint16_t value);
    uint32_t bauNumber();
    void bauNumber(uint32_t value);
    const uint8_t* orderNumber();
    void orderNumber(const uint8_t* value);
    const uint8_t* hardwareType();
    void hardwareType(const uint8_t* value);
    uint16_t version();
    void version(uint16_t value);
    uint16_t maskVersion();
    void maskVersion(uint16_t value);
    uint16_t maxApduLength();
    void maxApduLength(uint16_t value);
    const uint8_t* rfDomainAddress();
    void rfDomainAddress(uint8_t* value);
    uint8_t defaultHopCount();
private:
    uint8_t _prgMode = 0;
#if MASK_VERSION == 0x091A || MASK_VERSION == 0x2920
    uint16_t _ownAddress = 0xFF00; // 15.15.0; couplers have 15.15.0 as default PA
#else
    uint16_t _ownAddress = 0xFFFF; // 15.15.255;
#endif
};
