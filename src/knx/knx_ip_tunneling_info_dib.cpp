#include "knx_ip_tunneling_info_dib.h"
#include "service_families.h"
#if KNX_SERVICE_FAMILY_CORE >= 2

#ifdef USE_IP
KnxIpTunnelingInfoDIB::KnxIpTunnelingInfoDIB(uint8_t* data) : KnxIpDIB(data)
{
    currentPos = data + 4;
}

uint16_t KnxIpTunnelingInfoDIB::apduLength()
{
    uint16_t addr = 0;
    popWord(addr, _data+2);
    return addr;
}

void KnxIpTunnelingInfoDIB::apduLength(uint16_t addr)
{
    pushWord(addr, _data+2);
}

void KnxIpTunnelingInfoDIB::tunnelingSlot(uint16_t addr, uint16_t state)
{
    pushWord(addr, currentPos);
    pushWord(state, currentPos + 2);
    currentPos += 4;
    length(currentPos - _data);
}
#endif
#endif