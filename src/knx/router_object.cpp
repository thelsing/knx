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
