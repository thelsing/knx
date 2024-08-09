#pragma once

#include "config.h"
#ifdef USE_TP

#include "data_link_layer.h"
#include "tp_frame.h"
#include <stdint.h>

#define MAX_KNX_TELEGRAM_SIZE 263

#ifndef MAX_RX_QUEUE_BYTES
#define MAX_RX_QUEUE_BYTES MAX_KNX_TELEGRAM_SIZE + 50
#endif

#ifndef MAX_TX_QUEUE
#define MAX_TX_QUEUE 50
#endif

// __time_critical_func fallback
#ifndef ARDUINO_ARCH_RP2040
#define __time_critical_func(X) X
#define __isr
#endif

void printFrame(TpFrame* tpframe);

class ITpUartCallBacks
{
  public:
    virtual ~ITpUartCallBacks() = default;
    virtual TPAckType isAckRequired(uint16_t address, bool isGrpAddr) = 0;
};

class TpUartDataLinkLayer : public DataLinkLayer
{
    using DataLinkLayer::_deviceObject;
    using DataLinkLayer::_platform;

  public:
    TpUartDataLinkLayer(DeviceObject& devObj, NetworkLayerEntity& netLayerEntity,
                        Platform& platform, BusAccessUnit& busAccessUnit, ITpUartCallBacks& cb, DataLinkLayerCallbacks* dllcb = nullptr);

    void loop();
    void enabled(bool value);
    bool enabled() const;
    DptMedium mediumType() const override;
    bool reset();
    void monitor();
    void stop(bool state);
    void requestBusy(bool state);
    void forceAck(bool state);
    void setRepetitions(uint8_t nack, uint8_t busy);
    // Alias
    void setFrameRepetition(uint8_t nack, uint8_t busy);
    bool isConnected();
    bool isMonitoring();
    bool isStopped();
    bool isBusy();
    void resetStats();

#ifdef USE_TP_RX_QUEUE
    void processRxISR();
#endif
#ifdef NCN5120
    void powerControl(bool state);
#endif

    uint32_t getRxInvalidFrameCounter();
    uint32_t getRxProcessdFrameCounter();
    uint32_t getRxIgnoredFrameCounter();
    uint32_t getRxUnknownControlCounter();
    uint32_t getTxFrameCounter();
    uint32_t getTxProcessedFrameCounter();
    uint8_t getMode();

  private:
    // Frame
    struct knx_tx_queue_entry_t
    {
        TpFrame* frame;
        knx_tx_queue_entry_t* next = nullptr;

        knx_tx_queue_entry_t(TpFrame* tpFrame)
            : frame(tpFrame)
        {
        }
    };

    // TX Queue
    struct knx_tx_queue_t
    {
        knx_tx_queue_entry_t* front = nullptr;
        knx_tx_queue_entry_t* back = nullptr;
    } _txFrameQueue;

    TpFrame* _txFrame = nullptr;
    TpFrame* _rxFrame = nullptr;

    volatile bool _stopped = false;
    volatile bool _connected = false;
    volatile bool _monitoring = false;
    volatile bool _busy = false;
    volatile bool _initialized = false;

    volatile uint8_t _rxState = 0;
    volatile uint8_t _txState = 0;
    volatile uint32_t _rxProcessdFrameCounter = 0;
    volatile uint32_t _rxInvalidFrameCounter = 0;
    volatile uint32_t _rxIgnoredFrameCounter = 0;
    volatile uint32_t _rxUnkownControlCounter = 0;
    volatile uint32_t _txFrameCounter = 0;
    volatile uint32_t _txProcessdFrameCounter = 0;
    volatile bool _rxMarker = false;
    volatile bool _rxOverflow = false;
    volatile uint8_t _tpState = 0x0;
    volatile uint32_t _txLastTime = 0;
    volatile uint32_t _rxLastTime = 0;
    volatile bool _forceAck = false;
    uint8_t _txQueueCount = 0;

    inline bool markerMode();

    /*
     * bits
     *
     * 5-7 Busy (Default 11 = 3)
     * 0-3 Nack (Default 11 = 3)
     */
    volatile uint8_t _repetitions = 0b00110011;

    // to prevent parallel rx processing by isr (when using)
    volatile bool _rxProcessing = false;

    volatile uint32_t _lastStateRequest = 0;

    // void loadNextTxFrame();
    inline bool processTxFrameBytes();
    bool sendFrame(CemiFrame& frame);
    void rxFrameReceived(TpFrame* frame);
    void dataConBytesReceived(uint8_t* buffer, uint16_t length, bool success);

    void processRx(bool isr = false);
    void checkConnected();
    void processRxByte();
    void processTxQueue();
    void clearTxFrameQueue();
    void processRxFrameComplete();
    inline void processRxFrame(TpFrame* tpFrame);
    void pushTxFrameQueue(TpFrame* tpFrame);
    void requestState(bool force = false);
    void requestConfig();
    inline void processRxFrameByte(uint8_t byte);

#ifdef USE_TP_RX_QUEUE
    // Es muss ein Extended Frame rein passen + 1Byte je erlaubter ms Verzögerung
    volatile uint8_t _rxBuffer[MAX_RX_QUEUE_BYTES] = {};
    volatile uint16_t _rxBufferFront = 0;
    volatile uint16_t _rxBufferRear = 0;
    volatile uint8_t _rxBufferCount = 0;

    void pushByteToRxQueue(uint8_t byte);
    uint8_t pullByteFromRxQueue();
    uint16_t availableInRxQueue();
    void pushRxFrameQueue();
    void processRxQueue();
#endif

    inline bool isrLock(bool blocking = false);
    inline void isrUnlock();
    inline void clearUartBuffer();
    inline void connected(bool state = true);
    void clearTxFrame();
    void clearOutdatedTxFrame();
    void processTxFrameComplete(bool success);

    ITpUartCallBacks& _cb;
    DataLinkLayerCallbacks* _dllcb;
};
#endif
