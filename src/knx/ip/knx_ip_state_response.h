#pragma once

#include "knx_ip_frame.h"

class KnxIpStateResponse : public KnxIpFrame
{
    public:
        KnxIpStateResponse(uint8_t channelId, uint8_t errorCode);
    private:
};
