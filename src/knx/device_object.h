#pragma once

#include "interface_object.h"

class DeviceObject: public InterfaceObject
{
public:
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    void writeProperty(PropertyID id, uint32_t start, uint8_t* data, uint32_t& count);
    uint8_t propertySize(PropertyID id);
    uint8_t* save(uint8_t* buffer);
    uint8_t* restore(uint8_t* buffer);
    void readPropertyDescription(uint8_t propertyId, uint8_t& propertyIndex, bool& writeEnable, uint8_t& type, uint16_t& numberOfElements, uint8_t& access);
    ObjectType objectType() { return OT_DEVICE; }

    uint16_t induvidualAddress();
    void induvidualAddress(uint16_t value);
    bool userStopped();
    void userStopped(bool value);
    bool induvidualAddressDuplication();
    void induvidualAddressDuplication(bool value);
    bool verifyMode();
    void verifyMode(bool value);
    bool safeState();
    void safeState(bool value);
    bool progMode();
    void progMode(bool value);
    uint16_t manufacturerId();
    void manufacturerId(uint16_t value);
    uint32_t bauNumber();
    void bauNumber(uint32_t value);
    const uint8_t* knxSerialNumber();
    void knxSerialNumber(const uint8_t* value);
    const char* orderNumber();
    void orderNumber(const char* value);
    const uint8_t* hardwareType();
    void hardwareType(const uint8_t* value);
    uint16_t version();
    void version(uint16_t value);
    uint16_t maskVersion();
    void maskVersion(uint16_t value);
    uint16_t maxApduLength();
    void maxApduLength(uint16_t value);
    const uint32_t* ifObj();
    void ifObj(const uint32_t* value);
    uint8_t* rfDomainAddress();
    void rfDomainAddress(uint8_t* value);
protected:
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();
private:
    uint8_t _deviceControl = 0;
    uint8_t _routingCount = 0;
    uint8_t _prgMode = 0;
    uint16_t _ownAddress = 0;
    uint8_t _knxSerialNumber[6] = { 0x00, 0xFA, 0x00, 0x00, 0x00, 0x00 };  //Default to KNXA (0xFA)
    char _orderNumber[10] = "";
    uint8_t _hardwareType[6] = { 0, 0, 0, 0, 0, 0};
    uint16_t _version = 0;
    uint16_t _maskVersion = 0x0000;
    uint16_t _maxApduLength = 254;
    const uint32_t* _ifObjs;
    uint8_t _rfDomainAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
};