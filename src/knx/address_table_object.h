#pragma once

#include "table_object.h"

class AddressTableObject: public TableObject
{
public:
    AddressTableObject(Platform& platform);
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t *data);
    uint16_t entryCount();
    uint16_t getGa(uint16_t tsap);
    uint16_t getTsap(uint16_t ga);
    uint8_t* save(uint8_t* buffer);
    uint8_t* restore(uint8_t* buffer);
    bool contains(uint16_t addr);
protected:
    virtual void beforeStateChange(LoadState& newState);
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();
private:
    uint16_t* _groupAddresses;
};
