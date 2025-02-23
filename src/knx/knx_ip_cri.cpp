#include "knx_ip_cri.h"
#ifdef USE_IP
KnxIpCRI::KnxIpCRI(uint8_t* data) : _data(data)
{}

KnxIpCRI::~KnxIpCRI()
{}

uint8_t KnxIpCRI::length() const
{
    return *_data;
}

void KnxIpCRI::length(uint8_t value)
{
    *_data = value;
}

ConnectionType KnxIpCRI::type() const
{
    return (ConnectionType)_data[1];
}

void KnxIpCRI::type(ConnectionType value)
{
    _data[1] = value;
}

uint8_t KnxIpCRI::layer() const
{
    return _data[2];
}

void KnxIpCRI::layer(uint8_t value)
{
    _data[2] = value;
}
#endif