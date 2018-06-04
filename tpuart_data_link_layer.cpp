#include "tpuart_data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"
#include "cemi_frame.h"

#include <stdio.h>
#include <string.h>

TpUartDataLinkLayer::TpUartDataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab,
    NetworkLayer& layer, Platform& platform) : DataLinkLayer(devObj, addrTab, layer, platform)
{
}


bool TpUartDataLinkLayer::sendFrame(CemiFrame& frame)
{
    uint16_t length = frame.totalLenght();
    uint8_t* buffer = new uint8_t[length];

    //TODO: Create TP standard frame or extended frame from cemi frame into buffer
    
    bool success = sendBytes(buffer, length);

    delete[] buffer;
    return success;
}

void TpUartDataLinkLayer::loop()
{
    if (!_enabled)
        return;

    uint8_t buffer[512];
    int len = 0;
    // TODO: implement receiving of frames

    CemiFrame frame(buffer, len);
    frameRecieved(frame);
}

void TpUartDataLinkLayer::enabled(bool value)
{
    if (value && !_enabled)
    {
        //TODO: implement setup of Serial + TPUART
        _enabled = true;
        return;
    }

    if(!value && _enabled)
    {
        //TODO: implement disable of TPUART
        _enabled = false;
        return;
    }
}

bool TpUartDataLinkLayer::enabled() const
{
    return _enabled;
}


bool TpUartDataLinkLayer::sendBytes(uint8_t* bytes, uint16_t length)
{
    if (!_enabled)
        return false;

    //TODO: implement
    return false;
}