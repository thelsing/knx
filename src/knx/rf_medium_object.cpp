#include "rf_medium_object.h"

#include "bits.h"
#include "config.h"

#include <cstring>

#ifdef USE_RF

void RfMediumObject::readProperty(PropertyID propertyId, uint16_t start, uint8_t& count, uint8_t* data)
{
    switch (propertyId)
    {
        case PID_OBJECT_TYPE:
            pushWord(OT_RF_MEDIUM, data);
            break;
        case PID_RF_MULTI_TYPE:
            data[0] = 0x00; // KNX RF ready only
            break;
        case PID_RF_DOMAIN_ADDRESS:
            pushByteArray((uint8_t*)_rfDomainAddress, 6, data);
            break;
        case PID_RF_RETRANSMITTER:
            data[0] = 0x00; // No KNX RF retransmitter
            break;
        case PID_RF_BIDIR_TIMEOUT: // PDT_FUNCTION
            data[0] = 0x00; // success
            data[1] = 0xFF; // permanent bidirectional device
            data[2] = 0xFF; // permanent bidirectional device
            break;
        case PID_RF_DIAG_SA_FILTER_TABLE: // PDT_GENERIC_03[]
            pushByteArray((uint8_t*)_rfDiagSourceAddressFilterTable, 24, data);
            break;
        case PID_RF_DIAG_BUDGET_TABLE:
            pushByteArray((uint8_t*)_rfDiagLinkBudgetTable, 24, data);
            break;
        case PID_RF_DIAG_PROBE: // PDT_FUNCTION
            // Not supported yet
            break;
        default:
            count = 0;
    }
}

void RfMediumObject::writeProperty(PropertyID id, uint16_t start, uint8_t* data, uint8_t& count)
{
    switch (id)
    {
        case PID_RF_MULTI_TYPE:
            // We only support RF ready and not RF multi, ignore write request
        break;
        case PID_RF_DOMAIN_ADDRESS:
            for (uint8_t i = start; i < start + count; i++)
                _rfDomainAddress[i-1] = data[i - start];
        break;
        case PID_RF_BIDIR_TIMEOUT: // PDT_FUNCTION
            // Not supported yet (permanent bidir device)
        break;
        case PID_RF_DIAG_SA_FILTER_TABLE:
            for (uint8_t i = start; i < start + count; i++)
                _rfDiagSourceAddressFilterTable[i-1] = data[i - start];
        break;
        case PID_RF_DIAG_BUDGET_TABLE:
            for (uint8_t i = start; i < start + count; i++)
                _rfDiagLinkBudgetTable[i-1] = data[i - start];
        break;
        case PID_RF_DIAG_PROBE:
            // Not supported yet
        break;
        default:
            count = 0;
        break;
    }
}

uint8_t RfMediumObject::propertySize(PropertyID id)
{
    switch (id)
    {
    case PID_RF_MULTI_TYPE:
    case PID_RF_RETRANSMITTER:
        return 1;
    case PID_OBJECT_TYPE:
        return 2;
    case PID_RF_DOMAIN_ADDRESS:
        return 6;
    case PID_RF_DIAG_SA_FILTER_TABLE:
    case PID_RF_DIAG_BUDGET_TABLE:
        return 24;
    // case PID_RF_BIDIR_TIMEOUT: ?
    // case PID_RF_DIAG_PROBE: ?
    default:
        break;
    }
    return 0;
}

uint8_t* RfMediumObject::save(uint8_t* buffer)
{
    buffer = pushByteArray((uint8_t*)_rfDomainAddress, 6, buffer);
    return buffer;
}

const uint8_t* RfMediumObject::restore(const uint8_t* buffer)
{
    buffer = popByteArray((uint8_t*)_rfDomainAddress, 6, buffer);
    return buffer;
}

uint16_t RfMediumObject::saveSize()
{
    return 6;
}

uint8_t* RfMediumObject::rfDomainAddress()
{
    return _rfDomainAddress;
}

void RfMediumObject::rfDomainAddress(const uint8_t* value)
{
    pushByteArray(value, 6, _rfDomainAddress);
}

static PropertyDescription _propertyDescriptions[] = 
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_RF_MULTI_TYPE, false, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0 },
    { PID_RF_RETRANSMITTER, false, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0 },
    { PID_RF_DOMAIN_ADDRESS, true, PDT_GENERIC_06, 1, ReadLv3 | WriteLv0 }
};
static uint8_t _propertyDescriptionCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t RfMediumObject::propertyDescriptionCount()
{
    return _propertyDescriptionCount;
}

PropertyDescription* RfMediumObject::propertyDescriptions()
{
    return _propertyDescriptions;
}
#endif