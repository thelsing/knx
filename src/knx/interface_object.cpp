#include "interface_object.h"

void InterfaceObject::readPropertyDescription(uint8_t& propertyId, uint8_t& propertyIndex, bool& writeEnable, uint8_t& type, uint16_t& numberOfElements, uint8_t& access)
{
    PropertyDescription* descriptions = propertyDescriptions();
    uint8_t count = propertyCount();

    numberOfElements = 0;
    if (descriptions == nullptr || count == 0)
        return;

    PropertyDescription* desc = nullptr;

    // from KNX spec. 03.03.07 Application Layer (page 56) - 3.4.3.3  A_PropertyDescription_Read-service
    // Summary: either propertyId OR propertyIndex, but not both at the same time
    if (propertyId != 0)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            PropertyDescription d = descriptions[i];
            if (d.Id != propertyId)
                continue;

            desc = &d;
            propertyIndex = i;
            break;
        }
    }
    else
    {
        // If propertyId is zero, propertyIndex shall be used.
        // Response: propertyIndex of received A_PropertyDescription_Read
        if (propertyIndex < count)
        {
            desc = &descriptions[propertyIndex];
        }
    }

    if (desc != nullptr)
    {
        propertyId = desc->Id;
        writeEnable = desc->WriteEnable;
        type = desc->Type;
        numberOfElements = desc->MaxElements;
        access = desc->Access;
    }
}

void InterfaceObject::readProperty(PropertyID id, uint32_t start, uint32_t &count, uint8_t *data)
{
    count = 0;
}

void InterfaceObject::writeProperty(PropertyID id, uint8_t start, uint8_t *data, uint8_t count)
{
}

uint8_t InterfaceObject::propertySize(PropertyID id)
{
    return 0;
}

uint8_t InterfaceObject::propertyCount()
{
    return 0;
}

PropertyDescription* InterfaceObject::propertyDescriptions()
{
    return nullptr;
}

