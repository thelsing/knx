#include "config.h"
#ifdef USE_CEMI_SERVER

#include <cstring>
#include "cemi_server_object.h"
#include "bits.h"

void CemiServerObject::readProperty(PropertyID propertyId, uint16_t start, uint8_t& count, uint8_t* data)
{
    switch (propertyId)
    {
        case PID_OBJECT_TYPE:
            pushWord(OT_CEMI_SERVER, data);
            break;
        case PID_MEDIUM_TYPE: // PDT_BITSET16 
#if MEDIUM_TYPE==0        
            pushWord(2, data);  // TP1 supported
#elif MEDIUM_TYPE==2
            pushWord(16, data); // RF supported
#elif MEDIUM_TYPE==5
            pushWord(32, data); // IP supported
#endif
            break; 
        case PID_COMM_MODE: // PDT_ENUM8
            // See KNX spec. cEMI 3/6/3 p.110 
            data[0] = 0x00; // Only Data Link Layer mode supported and we do not allow switching (read-only)
            break;
        case PID_COMM_MODES_SUPPORTED:
            data[0] = 0x00;
            data[1] = 0x01;
            break;
        case PID_MEDIUM_AVAILABILITY: // PDT_BITSET16 
#if MEDIUM_TYPE==0        
            pushWord(2, data);  // TP1 active
#elif MEDIUM_TYPE==2
            pushWord(16, data); // RF active
#elif MEDIUM_TYPE==5
            pushWord(32, data); // IP active
#endif
            break;
        case PID_ADD_INFO_TYPES: // PDT_ENUM8[] 
            pushByteArray((uint8_t*)_addInfoTypesTable, sizeof(_addInfoTypesTable), data);
            break;
            // Not supported yet
            break;
        default:
            count = 0;
    }
}

void CemiServerObject::writeProperty(PropertyID id, uint16_t start, uint8_t* data, uint8_t& count)
{
    switch (id)
    {
        case PID_COMM_MODE:
            //_commMode = data[0]; // TODO: only Data Link Layer supported for now
            // Property is also marked as read-only, normally it is read/write.
        break;

        default:
            count = 0;
        break;
    }
}

uint8_t CemiServerObject::propertySize(PropertyID id)
{
    switch (id)
    {
    case PID_COMM_MODE:
        return 1;
    case PID_OBJECT_TYPE:
    case PID_MEDIUM_TYPE:
    case PID_MEDIUM_AVAILABILITY:
    case PID_COMM_MODES_SUPPORTED:
        return 2;
    case PID_ADD_INFO_TYPES:
        return sizeof(_addInfoTypesTable);
    default:
        break;
    }
    return 0;
}

static PropertyDescription _propertyDescriptions[] = 
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_MEDIUM_TYPE, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0 },
    { PID_COMM_MODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0 },
    { PID_COMM_MODES_SUPPORTED, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0 },
    { PID_MEDIUM_AVAILABILITY, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0 },
    { PID_ADD_INFO_TYPES, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0 }
};
static uint8_t _propertyDescriptionCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t CemiServerObject::propertyDescriptionCount()
{
    return _propertyDescriptionCount;
}

PropertyDescription* CemiServerObject::propertyDescriptions()
{
    return _propertyDescriptions;
}

#endif
