#pragma once

#include <stdint.h>

#define MAX_KNX_TELEGRAM_SIZE 263

class CemiServer;
class CemiFrame;

class UsbTunnelInterface
{
  public:
    UsbTunnelInterface(CemiServer& cemiServer, uint16_t manufacturerId, uint16_t maskVersion);

    void loop();

    // from cEMI server
    bool sendCemiFrame(CemiFrame& frame);

  private:
    struct _tx_queue_frame_t
    {
        uint8_t* data;
        uint16_t length;
        _tx_queue_frame_t* next;
    };

    struct _tx_queue_t
    {
        _tx_queue_frame_t* front = nullptr;
        _tx_queue_frame_t* back = nullptr;
    } _tx_queue;

    CemiServer& _cemiServer;

    void addFrameTxQueue(CemiFrame& frame);
    bool isTxQueueEmpty();
    void loadNextTxFrame(uint8_t** sendBuffer, uint16_t* sendBufferLength);
};
