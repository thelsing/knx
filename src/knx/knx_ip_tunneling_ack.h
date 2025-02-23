#pragma once

#include "cemi_frame.h"
#include "knx_ip_ch.h"
#include "knx_ip_frame.h"
#ifdef USE_IP

class KnxIpTunnelingAck : public KnxIpFrame
{
    public:
        KnxIpTunnelingAck(uint8_t* data, uint16_t length);
        KnxIpTunnelingAck();
        KnxIpCH& connectionHeader();

    private:
        KnxIpCH _ch;
};
#endif