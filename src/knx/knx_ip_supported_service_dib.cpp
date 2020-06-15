#include "knx_ip_supported_service_dib.h"

#ifdef USE_IP
KnxIpSupportedServiceDIB::KnxIpSupportedServiceDIB(uint8_t* data) : KnxIpDIB(data)
{}


uint8_t KnxIpSupportedServiceDIB::serviceVersion(ServiceFamily family)
{
    uint8_t* start = _data + 2;
    uint8_t* end = _data + length();

    for (uint8_t* it = start; it < end; it += 2)
    {
        if (*it == family)
            return it[1];
    }
    return 0;
}


void KnxIpSupportedServiceDIB::serviceVersion(ServiceFamily family,  uint8_t version)
{
    uint8_t* start = _data + 2;
    uint8_t* end = _data + length();

    for (uint8_t* it = start; it < end; it += 2)
    {
        if (*it == family)
        {
            it[1] = version;
            break;
        }

        if (*it == 0)
        {
            *it = family;
            it[1] = version;
            break;
        }
    }
}
#endif