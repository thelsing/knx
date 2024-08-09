#include "knx_ip_crd.h"
#ifdef USE_IP
KnxIpCRD::KnxIpCRD(uint8_t* data) : _data(data)
{}

KnxIpCRD::~KnxIpCRD()
{}

uint8_t KnxIpCRD::length() const
{
    return *_data;
}

void KnxIpCRD::length(uint8_t value)
{
    *_data = value;
}

uint8_t KnxIpCRD::type() const
{
    return _data[1];
}

void KnxIpCRD::type(uint8_t value)
{
    _data[1] = value;
}

uint16_t KnxIpCRD::address() const
{
    uint16_t addr = _data[3];
    addr |= _data[2] << 8;
    return addr;
}

void KnxIpCRD::address(uint16_t value)
{
    _data[2] = value >> 8;
    _data[3] = value & 0xFF;
}
#endif