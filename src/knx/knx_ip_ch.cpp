#include "knx_ip_ch.h"
#ifdef USE_IP
KnxIpCH::KnxIpCH(uint8_t* data) : _data(data)
{}

KnxIpCH::~KnxIpCH()
{}

uint8_t KnxIpCH::length() const
{
    return *_data;
}

void KnxIpCH::length(uint8_t value)
{
    *_data = value;
}

void KnxIpCH::channelId(uint8_t value)
{
    _data[1] = value;
}

uint8_t KnxIpCH::channelId() const
{
    return _data[1];
}

void KnxIpCH::sequenceCounter(uint8_t value)
{
    _data[2] = value;
}

uint8_t KnxIpCH::sequenceCounter() const
{
    return _data[2];
}

void KnxIpCH::status(uint8_t value)
{
    _data[3] = value;
}

uint8_t KnxIpCH::status() const
{
    return _data[3];
}
#endif