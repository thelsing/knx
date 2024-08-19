#pragma once

#include "knx_ip_frame.h"
#include "knx_ip_ch.h"
#include "../datalink_layer/cemi_frame.h"
class KnxIpTunnelingAck : public KnxIpFrame
{
    public:
        KnxIpTunnelingAck(uint8_t* data, uint16_t length);
        KnxIpTunnelingAck();
        KnxIpCH& connectionHeader();
    private:
        KnxIpCH _ch;
};
