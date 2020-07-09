#pragma once

#include "config.h"
#ifdef USE_RF

#include <stdint.h>
#include "data_link_layer.h"

#define MAX_KNX_TELEGRAM_SIZE 263

class RfPhysicalLayer;
class RfMediumObject;

class RfDataLinkLayer : public DataLinkLayer
{
    friend class RfPhysicalLayer;

    using DataLinkLayer::_deviceObject;
    using DataLinkLayer::_platform;

  public:
    RfDataLinkLayer(DeviceObject& devObj, RfMediumObject& rfMediumObj, NetworkLayer& layer,
                    Platform& platform);

    void loop();
    void enabled(bool value);
    bool enabled() const;

  private:
    bool _enabled = false;
    uint8_t _loopState = 0;

    uint8_t _buffer[512];

    uint8_t _frameNumber = 0;

    struct _tx_queue_frame_t
    {
        uint8_t* data;
        uint16_t length;
        _tx_queue_frame_t* next;
    };

    struct _tx_queue_t
    {
        _tx_queue_frame_t* front = NULL;
        _tx_queue_frame_t* back = NULL;
    } _tx_queue;

    RfMediumObject& _rfMediumObj;
    RfPhysicalLayer _rfPhy;

    void fillRfFrame(CemiFrame& frame, uint8_t* data);
    void addFrameTxQueue(CemiFrame& frame);
    bool isTxQueueEmpty();
    void loadNextTxFrame(uint8_t** sendBuffer, uint16_t* sendBufferLength);
    bool sendFrame(CemiFrame& frame);
    void frameBytesReceived(uint8_t* buffer, uint16_t length);
    uint16_t calcCrcRF(uint8_t* buffer, uint32_t offset, uint32_t len);
};

#endif
