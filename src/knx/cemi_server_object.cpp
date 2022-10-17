#include "config.h"
#ifdef USE_CEMI_SERVER

#include <cstring>
#include "cemi_server_object.h"
#include "bits.h"
#include "data_property.h"

CemiServerObject::CemiServerObject()
{
    Property* properties[] =
    {
        new DataProperty( PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t)OT_CEMI_SERVER ),
        new DataProperty( PID_MEDIUM_TYPE, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0, (uint16_t)0),
        new DataProperty( PID_COMM_MODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0, (uint16_t)0),
        new DataProperty( PID_COMM_MODES_SUPPORTED, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0, (uint16_t)0x100),
        new DataProperty( PID_MEDIUM_AVAILABILITY, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0, (uint16_t)0),
        // cEMI additional info types supported by this cEMI server: only 0x02 (RF Control Octet and Serial Number or DoA)
        new DataProperty( PID_ADD_INFO_TYPES, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0, (uint8_t)0x02)
    };
    initializeProperties(sizeof(properties), properties);
}

void CemiServerObject::setMediumTypeAsSupported(DptMedium dptMedium)
{
    uint16_t mediaTypesSupported;
    property(PID_MEDIUM_TYPE)->read(mediaTypesSupported);

    switch(dptMedium)
    {
    case DptMedium::KNX_IP:
        mediaTypesSupported |= 1 << 1;
        break;
    case DptMedium::KNX_RF:
        mediaTypesSupported |= 1 << 4;
        break;
    case DptMedium::KNX_TP1:
        mediaTypesSupported |= 1 << 5;
        break;
    case DptMedium::KNX_PL110:
        mediaTypesSupported |= 1 << 2;
        break;
    }

    property(PID_MEDIUM_TYPE)->write(mediaTypesSupported);
    // We also set the medium as available too
    property(PID_MEDIUM_AVAILABILITY)->write(mediaTypesSupported);
}

void CemiServerObject::clearSupportedMediaTypes()
{
    property(PID_MEDIUM_TYPE)->write((uint16_t) 0);
    // We also set the medium as not available too
    property(PID_MEDIUM_AVAILABILITY)->write((uint16_t) 0);
}

#endif

