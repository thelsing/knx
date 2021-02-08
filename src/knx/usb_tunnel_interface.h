#pragma once

#include "config.h"
#ifdef USE_USB
#include <stdint.h>

class CemiServer;
class CemiFrame;

enum ProtocolIdType
{
	KnxTunneling = 0x01,
	BusAccessServer = 0x0f
};

enum EmiIdType
{
	EmiIdNotUsed = 0x00,
	EMI1 = 0x01,
	EMI2 = 0x02,
	CEMI = 0x03
};

enum ServiceIdType
{
	ServiceIdNotUsed = 0x00,
	DeviceFeatureGet = 0x01,
	DeviceFeatureResponse = 0x02,
	DeviceFeatureSet = 0x03,
	DeviceFeatureInfo = 0x04,
  DeviceFeatureEscape = 0xFF
};

enum FeatureIdType
{
  SupportedEmiType = 0x01,
  HostDeviceDescriptorType0 = 0x02,
  BusConnectionStatus = 0x03,
  KnxManufacturerCode = 0x04,
  ActiveEmiType = 0x05
};

class UsbTunnelInterface
{
  public:
    UsbTunnelInterface(CemiServer& cemiServer, uint16_t manufacturerId, uint16_t maskVersion);

    void loop();

    // from cEMI server
    void sendCemiFrame(CemiFrame& frame);

    static const uint8_t* getKnxHidReportDescriptor();
    static uint16_t getHidReportDescriptorLength();
    static void receiveHidReport(uint8_t const* data, uint16_t bufSize);

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
    void addBufferTxQueue(uint8_t* data, uint16_t length);
    bool isTxQueueEmpty();
    void loadNextTxFrame(uint8_t** sendBuffer, uint16_t* sendBufferLength);

    // USB RX queue
    static _queue_t _rx_queue;
    static void addBufferRxQueue(const uint8_t* data, uint16_t length);
    bool isRxQueueEmpty();
    void loadNextRxBuffer(uint8_t** receiveBuffer, uint16_t* receiveBufferLength);
    static bool rxHaveCompletePacket;

    void handleTransferProtocolPacket(uint8_t* data, uint16_t length);
    void handleHidReportRxQueue();
    void handleBusAccessServerProtocol(ServiceIdType servId, const uint8_t* requestData, uint16_t packetLength);
    void sendKnxHidReport(ProtocolIdType protId, ServiceIdType servId, uint8_t* data, uint16_t length);
};
#endif