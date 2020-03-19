#include "cemi_server_object.h"

#include "config.h"
#include "bits.h"
#include "data_property.h"

#include <cstring>

#ifdef USE_CEMI_SERVER

CemiServerObject::CemiServerObject()
{
    uint16_t mediumType = 0;
#if MEDIUM_TYPE == 0
    mediumType = 2; // TP1 supported
#elif MEDIUM_TYPE == 2
    mediumType = 16; // RF supported
#elif MEDIUM_TYPE == 5
    mediumType = 32; // IP supported
#endif

    Property* properties[] =
    {
        new DataProperty( PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t)OT_CEMI_SERVER ),
        new DataProperty( PID_MEDIUM_TYPE, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0, mediumType),
        new DataProperty( PID_COMM_MODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0, (uint16_t)0),
        new DataProperty( PID_COMM_MODES_SUPPORTED, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0, (uint16_t)0x100),
        new DataProperty( PID_MEDIUM_AVAILABILITY, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0, mediumType),
        // cEMI additional info types supported by this cEMI server: only 0x02 (RF Control Octet and Serial Number or DoA)
        new DataProperty( PID_ADD_INFO_TYPES, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0, (uint8_t)0x02)
    };
    initializeProperties(sizeof(properties), properties);
}
#endif

