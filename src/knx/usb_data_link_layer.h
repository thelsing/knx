#pragma once

#include <stdint.h>
#include <Adafruit_TinyUSB.h>
#include "data_link_layer.h"
#include "cemi_server.h"

#define MAX_KNX_TELEGRAM_SIZE 263

extern Adafruit_USBD_HID usb_hid;

class DeviceObject;
class Platform;

class UsbDataLinkLayer
{
  public:
    UsbDataLinkLayer(DeviceObject& deviceObject, CemiServer& cemiServer, Platform& platform);

    void loop();
    void enabled(bool value);
    bool enabled() const;

    bool sendCemiFrame(CemiFrame& frame);

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

    DeviceObject& _deviceObject;
    CemiServer& _cemiServer;
    Platform& _platform;

    void addFrameTxQueue(CemiFrame& frame);
    bool isTxQueueEmpty();
    void loadNextTxFrame(uint8_t** sendBuffer, uint16_t* sendBufferLength);
    void frameBytesReceived(uint8_t* buffer, uint16_t length);
};
