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

    static const uint8_t* getKnxHidReportDescriptor();
    static uint16_t getHidReportDescriptorLength();
    static void receiveKnxHidReport(uint8_t const* data, uint16_t bufSize);

  private:
    struct _queue_buffer_t
    {
        uint8_t* data;
        uint16_t length;
        _queue_buffer_t* next;
    };

    struct _queue_t
    {
        _queue_buffer_t* front = nullptr;
        _queue_buffer_t* back = nullptr;
    };

    static const uint8_t descHidReport[];

    CemiServer& _cemiServer;

    uint16_t _manufacturerId;
    uint16_t _maskVersion;

    // USB TX queue
    _queue_t _tx_queue;
    void addFrameTxQueue(CemiFrame& frame);
    bool isTxQueueEmpty();
    void loadNextTxFrame(uint8_t** sendBuffer, uint16_t* sendBufferLength);

    // USB RX queue
    static _queue_t _rx_queue;
    static void addBufferRxQueue(const uint8_t* data, uint16_t length);
    bool isRxQueueEmpty();
    void loadNextRxBuffer(uint8_t** receiveBuffer, uint16_t* receiveBufferLength);

    void handleKnxHidReport(uint8_t const* data, uint16_t bufSize);
    void handleBusAccessServerProtocol(const uint8_t* requestData, uint16_t packetLength);
    void sendKnxTunnelHidReport(uint8_t* data, uint16_t length);
};
