#include <cstring>
#include "cemi_server_object.h"
#include "bits.h"

void CemiServerObject::readProperty(PropertyID propertyId, uint32_t start, uint32_t& count, uint8_t* data)
{
    switch (propertyId)
    {
        case PID_OBJECT_TYPE:
            pushWord(OT_CEMI_SERVER, data);
            break;
        case PID_MEDIUM_TYPE: // PDT_BITSET16 
#if MEDIUM_TYPE==0        
            data[0] = 2;  // TP1 supported
#elif MEDIUM_TYPE==2
            data[0] = 16; // RF supported
#elif MEDIUM_TYPE==5
            data[0] = 32; // IP supported
#endif
            break; 
        case PID_COMM_MODE: // PDT_ENUM8
            // See KNX spec. cEMI 3/6/3 p.110 
            data[0] = 0x00; // Only Data Link Layer mode supported and we do not allow switching (read-only)
            break;
        case PID_MEDIUM_AVAILABILITY: // PDT_BITSET16 
#if MEDIUM_TYPE==0        
            data[0] = 2;  // TP1 active
#elif MEDIUM_TYPE==2
            data[0] = 16; // RF active
#elif MEDIUM_TYPE==5
            data[0] = 32; // IP active
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

void CemiServerObject::writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count)
{
    switch (id)
    {
        case PID_COMM_MODE:
            //_commMode = data[0]; // TODO: only Data Link Layer supported for now
            // Property is also marked as read-only, normally it is read/write.
        break;

        default:
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
        return 2;
    case PID_ADD_INFO_TYPES:
        return sizeof(_addInfoTypesTable);
    default:
        break;
    }
    return 0;
}

uint8_t* CemiServerObject::save(uint8_t* buffer)
{
    return buffer;
}

uint8_t* CemiServerObject::restore(uint8_t* buffer)
{
    return buffer;
}

static PropertyDescription _propertyDescriptions[] = 
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_MEDIUM_TYPE, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0 },
    { PID_COMM_MODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0 },
    { PID_MEDIUM_AVAILABILITY, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0 },
    { PID_ADD_INFO_TYPES, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0 }
};
static uint8_t _propertyCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t CemiServerObject::propertyCount()
{
    return _propertyCount;
}

PropertyDescription* CemiServerObject::propertyDescriptions()
{
    return _propertyDescriptions;
}
