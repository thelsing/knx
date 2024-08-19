#pragma once

#include "knx_ip_frame.h"
#include "knx_ip_ch.h"
#include "../datalink_layer/cemi_frame.h"
class KnxIpTunnelingRequest : public KnxIpFrame
{
    public:
        KnxIpTunnelingRequest(uint8_t* data, uint16_t length);
        KnxIpTunnelingRequest(CemiFrame frame);
        CemiFrame& frame();
        KnxIpCH& connectionHeader();
    private:
        CemiFrame _frame;
        KnxIpCH _ch;
};
