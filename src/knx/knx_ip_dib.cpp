#include "knx_ip_dib.h"
#ifdef USE_IP
KnxIpDIB::KnxIpDIB(uint8_t* data) : _data(data)
{}

KnxIpDIB::~KnxIpDIB()
{}

uint8_t KnxIpDIB::length() const
{
    return *_data;
}

void KnxIpDIB::length(uint8_t value)
{
    *_data = value;
}

DescriptionTypeCode KnxIpDIB::code() const
{
    return (DescriptionTypeCode)_data[1];
}

void KnxIpDIB::code(DescriptionTypeCode value)
{
    _data[1] = value;
}
#endif