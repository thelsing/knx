#include "knx_ip_config_dib.h"

#ifdef USE_IP
KnxIpConfigDIB::KnxIpConfigDIB(uint8_t* data, bool isCurrent) : KnxIpDIB(data)
{
    _isCurrent = isCurrent;
}

uint32_t KnxIpConfigDIB::address()
{
    uint32_t addr = 0;
    popInt(addr, _data + 2);
    return addr;
}

void KnxIpConfigDIB::address(uint32_t addr)
{
    pushInt(addr, _data + 2);
}

uint32_t KnxIpConfigDIB::subnet()
{
    uint32_t addr = 0;
    popInt(addr, _data + 6);
    return addr;
}

void KnxIpConfigDIB::subnet(uint32_t addr)
{
    pushInt(addr, _data + 6);
}

uint32_t KnxIpConfigDIB::gateway()
{
    uint32_t addr = 0;
    popInt(addr, _data + 10);
    return addr;
}

void KnxIpConfigDIB::gateway(uint32_t addr)
{
    pushInt(addr, _data + 10);
}

uint32_t KnxIpConfigDIB::dhcp()
{
    if(!_isCurrent) return 0;
    uint32_t addr = 0;
    popInt(addr, _data + 14);
    return addr;
}

void KnxIpConfigDIB::dhcp(uint32_t addr)
{
    if(!_isCurrent) return;
    pushInt(addr, _data + 14);
}

uint8_t KnxIpConfigDIB::info1()
{
    if(_isCurrent)
        return _data[14];
    else
        return _data[18];
}

void KnxIpConfigDIB::info1(uint8_t addr)
{
    if(_isCurrent)
        _data[14] = addr;
    else
        _data[18] = addr;
}

uint8_t KnxIpConfigDIB::info2()
{
    if(_isCurrent)
        return _data[15];
    else
        return _data[19];
}

void KnxIpConfigDIB::info2(uint8_t addr)
{
    if(_isCurrent)
        _data[15] = addr;
    else
        _data[19] = addr;
}

#endif