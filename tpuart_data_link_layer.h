#pragma once

#include <stdint.h>
#include "data_link_layer.h"

class TpUartDataLinkLayer: public DataLinkLayer
{
    using DataLinkLayer::_deviceObject;
    using DataLinkLayer::_platform;
    using DataLinkLayer::_groupAddressTable;
public:
    TpUartDataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, NetworkLayer& layer,
        Platform& platform);

    void loop();
    void enabled(bool value);
    bool enabled() const;
private:
    bool _enabled = false;
    uint8_t* _sendBuffer = 0;
    bool _sendResult = false;
    uint16_t _sendBufferLength = 0;
    bool sendFrame(CemiFrame& frame);
    bool checkDataInd(uint8_t firstByte);
    bool checkDataCon(uint8_t firstByte);
    bool checkPollDataInd(uint8_t firstByte);
    bool checkAckNackInd(uint8_t firstByte);
    bool checkResetInd(uint8_t firstByte);
    bool checkStateInd(uint8_t firstByte);
    bool checkFrameStateInd(uint8_t firstByte);
    bool checkConfigureInd(uint8_t firstByte);
    bool checkFrameEndInd(uint8_t firstByte);
    bool checkStopModeInd(uint8_t firstByte);
    bool checkSystemStatInd(uint8_t firstByte);
    void handleUnexpected(uint8_t firstByte);
    void sendBytes(uint8_t* buffer, uint16_t length);
    void frameBytesReceived(uint8_t* buffer, uint16_t length);
    void resetChip();
    void stopChip();
};