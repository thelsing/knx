#include "knx_ip_knx_addresses_dib.h"

#ifdef USE_IP
KnxIpKnxAddressesDIB::KnxIpKnxAddressesDIB(uint8_t* data) : KnxIpDIB(data)
{
    currentPos = data + 4;
}

uint16_t KnxIpKnxAddressesDIB::individualAddress()
{
    uint16_t addr = 0;
    popWord(addr, _data + 2);
    return addr;
}

void KnxIpKnxAddressesDIB::individualAddress(uint16_t addr)
{
    pushInt(addr, _data + 2);
}

void KnxIpKnxAddressesDIB::additional(uint16_t addr)
{
    pushWord(addr, currentPos);
    currentPos += 2;
    length(currentPos - _data);
}
#endif