#pragma once

#include "config.h"
#ifdef USE_TP

#include <stdint.h>
#include "data_link_layer.h"

#define MAX_KNX_TELEGRAM_SIZE 263

class ITpUartCallBacks
{
public:
    virtual ~ITpUartCallBacks() = default;
    virtual bool isAckRequired(uint16_t address, bool isGrpAddr) = 0;
};

class TpUartDataLinkLayer : public DataLinkLayer
{
    using DataLinkLayer::_deviceObject;
    using DataLinkLayer::_platform;

  public:
    TpUartDataLinkLayer(DeviceObject& devObj, NetworkLayerEntity& netLayerEntity,
                        Platform& platform, ITpUartCallBacks& cb, DataLinkLayerCallbacks* dllcb = nullptr);



    void loop();
    void enabled(bool value);
    bool enabled() const;
    DptMedium mediumType() const override;

  private:
    bool _enabled = false;
    uint8_t* _sendBuffer = 0;
    uint16_t _sendBufferLength = 0;
    uint8_t _receiveBuffer[MAX_KNX_TELEGRAM_SIZE];
    uint8_t _txState = 0;
    uint8_t _rxState = 0;
    uint16_t _RxByteCnt = 0;
    uint16_t _TxByteCnt = 0;
    uint8_t _oldIdx = 0;
    bool _isEcho = false;
    bool _convert = false;
    uint8_t _xorSum = 0;
    uint32_t _lastByteRxTime;
    uint32_t _lastByteTxTime;
    uint32_t _lastLoopTime;
    uint32_t _waitConfirmStartTime = 0;
    uint32_t _lastResetChipTime = 0;

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

    void addFrameTxQueue(CemiFrame& frame);
    bool isTxQueueEmpty();
    void loadNextTxFrame();
    bool sendSingleFrameByte();
    bool sendFrame(CemiFrame& frame);
    void frameBytesReceived(uint8_t* buffer, uint16_t length);
    void dataConBytesReceived(uint8_t* buffer, uint16_t length, bool success);
    void enterRxWaitEOP();
    bool resetChip();
    bool resetChipTick();
    void stopChip();

    ITpUartCallBacks& _cb;
    DataLinkLayerCallbacks* _dllcb;
};
#endif
