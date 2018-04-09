#pragma once

#include <stddef.h>
#include "property_types.h"
#include "save_restore.h"

enum ObjectType
{
    /** Device object. */
    OT_DEVICE = 0,

    /** Address table object. */
    OT_ADDR_TABLE = 1,

    /** Association table object. */
    OT_ASSOC_TABLE = 2,

    /** Application program object. */
    OT_APPLICATION_PROG = 3,

    /** Interface program object. */
    OT_INTERFACE_PROG = 4,

    /** KNX - Object Associationtable. */
    OT_OJB_ASSOC_TABLE = 5,

    /** Router Object */
    OT_ROUTER = 6,

    /** LTE Address Routing Table Object */
    OT_LTE_ADDR_ROUTING_TABLE = 7,

    /** cEMI Server Object */
    OT_CEMI_SERVER = 8,

    /** Group Object Table Object */
    OT_GRP_OBJ_TABLE = 9,

    /** Polling Master */
    OT_POLLING_MASTER = 10,

    /** KNXnet/IP Parameter Object */
    OT_IP_PARAMETER = 11,

    /** Reserved. Shall not be used. */
    OT_RESERVED = 12,

    /** File Server Object */
    OT_FILE_SERVER = 13
};


class InterfaceObject: public SaveRestore
{
public:
    virtual ~InterfaceObject() {}
    virtual void readProperty(PropertyID id, uint32_t start, uint32_t count, uint8_t* data) = 0;
    virtual void writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count) = 0;
    virtual uint8_t propertySize(PropertyID id) = 0;
protected:

};