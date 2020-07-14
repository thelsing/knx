#include "config.h"

#include <cstring>
#include "router_object.h"
#include "bits.h"
#include "data_property.h"

RouterObject::RouterObject()
{
    initializeProperties(0, nullptr);
}

void RouterObject::initializeProperties(size_t propertiesSize, Property** properties)
{
    Property* ownProperties[] =
    {
        new DataProperty( PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t) OT_ROUTER ),
        new DataProperty( PID_OBJECT_INDEX, false, PDT_UNSIGNED_CHAR, 1, ReadLv3 | WriteLv0, (uint16_t) 0 ), // TODO
        new DataProperty( PID_MEDIUM_STATUS, false, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0, (uint16_t) 0 ), // TODO
        new DataProperty( PID_MAX_APDU_LENGTH_ROUTER, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t) 254 ),
        new DataProperty( PID_HOP_COUNT, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t) 5),
        new DataProperty( PID_MEDIUM, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0, (uint16_t) 0) , // TODO
    };

    uint8_t ownPropertiesCount = sizeof(ownProperties) / sizeof(Property*);

    uint8_t propertyCount = propertiesSize / sizeof(Property*);
    uint8_t allPropertiesCount = propertyCount + ownPropertiesCount;

    Property* allProperties[allPropertiesCount];
    if (properties)
        memcpy(allProperties, properties, propertiesSize);
    memcpy(allProperties + propertyCount, ownProperties, sizeof(ownProperties));

    InterfaceObject::initializeProperties(sizeof(allProperties), allProperties);
}

void RouterObject::masterReset(EraseCode eraseCode, uint8_t channel)
{
    if (eraseCode == FactoryReset)
    {
        // TODO handle different erase codes
        println("Factory reset of router object requested.");
    }
}

uint16_t RouterObject::getNumberOfElements(PropertyID propId)
{
    // Get number of entries for this property
    uint16_t numElements = 0;

    uint8_t data[sizeof(uint16_t)]; // is sizeof(_currentElements) which is uint16_t
    uint8_t count = property(propId)->read(0, 1, data);

    if (count > 0)
    {
        popWord(numElements, data);
    }

    return numElements;
}
