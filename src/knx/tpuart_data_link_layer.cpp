#include "config.h"
#ifdef USE_TP

#include "tpuart_data_link_layer.h"
#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"
#include "cemi_frame.h"

#include <stdio.h>
#include <string.h>

// Activate trace output
//#define DBG_TRACE

// NCN5120
//#define NCN5120

// services Host -> Controller :
// internal commands, device specific
#define U_RESET_REQ          0x01
#define U_STATE_REQ          0x02
#define U_SET_BUSY_REQ       0x03
#define U_QUIT_BUSY_REQ      0x04
#define U_BUSMON_REQ         0x05
#define U_SET_ADDRESS_REQ    0xF1 // different on TP-UART
#define U_SET_REPETITION_REQ 0xF2
#define U_L_DATA_OFFSET_REQ  0x08 //-0x0C
#define U_SYSTEM_STATE       0x0D
#define U_STOP_MODE_REQ      0x0E
#define U_EXIT_STOP_MODE_REQ 0x0F
#define U_ACK_REQ            0x10 //-0x17
#define U_ACK_REQ_NACK       0x04
#define U_ACK_REQ_BUSY       0x02
#define U_ACK_REQ_ADRESSED   0x01
#define U_CONFIGURE_REQ      0x18
#define U_INT_REG_WR_REQ     0x28
#define U_INT_REG_RD_REQ     0x38
#define U_POLLING_STATE_REQ  0xE0

//knx transmit data commands
#define U_L_DATA_START_CONT_REQ 0x80 //-0xBF
#define U_L_DATA_END_REQ        0x40 //-0x7F

//serices to host controller

// DLL services (device is transparent)
#define L_DATA_STANDARD_IND 0x90
#define L_DATA_EXTENDED_IND 0x10
#define L_DATA_MASK         0xD3
#define L_POLL_DATA_IND     0xF0

// acknowledge services (device is transparent in bus monitor mode)
#define L_ACKN_IND          0x00
#define L_ACKN_MASK         0x33
#define L_DATA_CON          0x0B
#define L_DATA_CON_MASK     0x7F
#define SUCCESS             0x80

// control services, device specific
#define U_RESET_IND           0x03
#define U_STATE_IND           0x07
#define SLAVE_COLLISION       0x80
#define RECEIVE_ERROR         0x40
#define TRANSMIT_ERROR        0x20
#define PROTOCOL_ERROR        0x10
#define TEMPERATURE_WARNING   0x08
#define U_FRAME_STATE_IND     0x13
#define U_FRAME_STATE_MASK    0x17
#define PARITY_BIT_ERROR      0x80
#define CHECKSUM_LENGTH_ERROR 0x40
#define TIMING_ERROR          0x20
#define U_CONFIGURE_IND       0x01
#define U_CONFIGURE_MASK      0x83
#define AUTO_ACKNOWLEDGE      0x20
#define AUTO_POLLING          0x10
#define CRC_CCITT             0x80
#define FRAME_END_WITH_MARKER 0x40
#define U_FRAME_END_IND       0xCB
#define U_STOP_MODE_IND       0x2B
#define U_SYSTEM_STAT_IND     0x4B

//tx states
enum {
    TX_IDLE,
    TX_FRAME,
    TX_WAIT_CONN
};

//rx states
enum {
    RX_WAIT_START,
    RX_L_ADDR,
    RX_L_DATA,
    RX_WAIT_EOP
};

#define EOP_TIMEOUT           2   //milli seconds; end of layer-2 packet gap
#ifndef EOPR_TIMEOUT              // allow to set EOPR_TIMEOUT externally
#define EOPR_TIMEOUT          8   //ms; relaxed EOP timeout; usally to trigger after NAK
#endif
#define CONFIRM_TIMEOUT       500  //milli seconds
#define RESET_TIMEOUT         100 //milli seconds
#define TX_TIMEPAUSE            0 // 0 means 1 milli seconds

#ifndef OVERRUN_COUNT
#define OVERRUN_COUNT          7 //bytes; max. allowed bytes in receive buffer (on start) to see it as overrun
#endif

// If this threshold is reached loop() goes into 
// "hog mode" where it stays in loop() while L2 address reception
#define HOGMODE_THRESHOLD       3 // milli seconds

void TpUartDataLinkLayer::enterRxWaitEOP()
{
    // Flush input
    while (_platform.uartAvailable())
    {
        _platform.readUart();
    }
    _lastByteRxTime = millis();
    _rxState = RX_WAIT_EOP;
}

void TpUartDataLinkLayer::loop()
{
    if (!_enabled)
    {
        if(_waitConfirmStartTime == 0)
        {
            if (millis() - _lastResetChipTime > 1000)
            { 
                //reset chip every 1 seconds
                _lastResetChipTime = millis();
                _enabled = resetChip();
            }
        } else {
            _enabled = resetChipTick();
        }
    }

    if (!_enabled)
        return;

    // Loop once and repeat as long we have rx data available
    do {
        // Signals to communicate from rx part with the tx part
        uint8_t dataConnMsg = 0;  // The DATA_CONN message just seen or 0

#ifdef KNX_WAIT_FOR_ADDR
        // After seeing a L2 packet start, stay in loop until address bytes are
        // received and the AK/NAK packet is sent
        bool stayInRx = true;
#elif defined(KNX_AUTO_ADAPT)
        // After seeing a L2 packet start, stay in loop until address bytes are
        // received and the AK/NAK packet is sent, when last loop call delayed
        // by more than HOGMODE_THRESHOLD
        bool stayInRx = millis() - _lastLoopTime > HOGMODE_THRESHOLD;
        _lastLoopTime = millis();
#else
        // After seeing a L2 packet start, leave loop and hope the loop
        // is called early enough to do further processings
        bool stayInRx = false;
#endif

        // Loop once and repeat as long we are in the receive phase for the L2 address
        do {
            uint8_t* buffer = _receiveBuffer + 2;
            uint8_t rxByte;
            switch (_rxState)
            {
                case RX_WAIT_START:
                    if (_platform.uartAvailable())
                    {
                        if (_platform.uartAvailable() > OVERRUN_COUNT)
                        {
                            print("input buffer overrun: "); println(_platform.uartAvailable());
                            enterRxWaitEOP();
                            break;
                        }
                        rxByte = _platform.readUart();
#ifdef DBG_TRACE
                        print(rxByte, HEX);
#endif
                        _lastByteRxTime = millis();

                        // Check for layer-2 packets
                        _RxByteCnt = 0;
                        _xorSum = 0;
                        if ((rxByte & L_DATA_MASK) == L_DATA_STANDARD_IND)
                        {
                            buffer[_RxByteCnt++] = rxByte;
                            _xorSum ^= rxByte;
                            _RxByteCnt++; //convert to L_DATA_EXTENDED
                            _convert = true;
                            _rxState = RX_L_ADDR;
#ifdef DBG_TRACE
                            println("RLS");
#endif
                            break;
                        }
                        else if ((rxByte & L_DATA_MASK) == L_DATA_EXTENDED_IND)
                        {
                            buffer[_RxByteCnt++] = rxByte;
                            _xorSum ^= rxByte;
                            _convert = false;
                            _rxState = RX_L_ADDR;
#ifdef DBG_TRACE
                            println("RLX");
#endif
                            break;
                        }

                        // Handle all single byte packets here
                        else if ((rxByte & L_DATA_CON_MASK) == L_DATA_CON)
                        {
                            dataConnMsg = rxByte;
                        }
                        else if (rxByte == L_POLL_DATA_IND)
                        {
                            // not sure if this can happen
                            println("got L_POLL_DATA_IND");
                        }
                        else if ((rxByte & L_ACKN_MASK) == L_ACKN_IND)
                        {
                            // this can only happen in bus monitor mode
                            println("got L_ACKN_IND");
                        }
                        else if (rxByte == U_RESET_IND)
                        {
                            println("got U_RESET_IND");
                        }
                        else if ((rxByte & U_STATE_IND) == U_STATE_IND)
                        {
                            print("got U_STATE_IND:");
                            if (rxByte & 0x80) print (" SC");
                            if (rxByte & 0x40) print (" RE");
                            if (rxByte & 0x20) print (" TE");
                            if (rxByte & 0x10) print (" PE");
                            if (rxByte & 0x08) print (" TW");
                            println();
                        }
                        else if ((rxByte & U_FRAME_STATE_MASK) == U_FRAME_STATE_IND)
                        {
                            print("got U_FRAME_STATE_IND: 0x");
                            print(rxByte, HEX);
                            println();
                        }
                        else if ((rxByte & U_CONFIGURE_MASK) == U_CONFIGURE_IND)
                        {
                            print("got U_CONFIGURE_IND: 0x");
                            print(rxByte, HEX);
                            println();
                        }
                        else if (rxByte == U_FRAME_END_IND)
                        {
                            println("got U_FRAME_END_IND");
                        }
                        else if (rxByte == U_STOP_MODE_IND)
                        {
                            println("got U_STOP_MODE_IND");
                        }
                        else if (rxByte == U_SYSTEM_STAT_IND)
                        {
                            print("got U_SYSTEM_STAT_IND: 0x");
                            while (true)
                            {
                                int tmp = _platform.readUart();
                                if (tmp < 0)
                                    continue;

                                print(tmp, HEX);
                                break;
                            }
                            println();
                        }
                        else
                        {
                            print("got UNEXPECTED: 0x");
                            print(rxByte, HEX);
                            println();
                        }
                    }
                    break;
                case RX_L_ADDR:
                    if (millis() - _lastByteRxTime > EOPR_TIMEOUT)
                    {
                        _rxState = RX_WAIT_START;
                        println("EOPR @ RX_L_ADDR");
                        break;
                    }
                    if (!_platform.uartAvailable())
                        break;
                    _lastByteRxTime = millis();
                    rxByte = _platform.readUart();
#ifdef DBG_TRACE
                    print(rxByte, HEX);
#endif
                    buffer[_RxByteCnt++] = rxByte;
                    _xorSum ^= rxByte;

                    if (_RxByteCnt == 7)
                    {
                        //Destination Address + payload available
                        //check if echo; ignore repeat bit of control byte
                        _isEcho = (_sendBuffer != nullptr && (!((buffer[0] ^ _sendBuffer[0]) & ~0x20) && !memcmp(buffer + _convert + 1, _sendBuffer + 1, 5)));

                        //convert into Extended.ind
                        if (_convert)
                        {
                            buffer[1] = buffer[6] & 0xF0;
                            buffer[6] &= 0x0F;
                        }

                        if (!_isEcho)
                        {
                            uint8_t c = U_ACK_REQ;

                            // The bau knows everything and could either check the address table object (normal device)
                            // or any filter tables (coupler) to see if we are addressed.

                            //check if individual or group address
                            bool isGroupAddress = (buffer[1] & 0x80) != 0;
                            uint16_t addr = getWord(buffer + 4);

                            if (_cb.isAckRequired(addr, isGroupAddress))
                            {
                                c |= U_ACK_REQ_ADRESSED;
                            }

                            // Hint: We can send directly here, this doesn't disturb other transmissions
                            // We don't have to update _lastByteTxTime because after U_ACK_REQ the timing is not so tight
                            _platform.writeUart(c);
                        }
                        _rxState = RX_L_DATA;
                    }
                    break;
                case RX_L_DATA:
                    if (!_platform.uartAvailable())
                        break;
                    _lastByteRxTime = millis();
                    rxByte = _platform.readUart();
#ifdef DBG_TRACE
                    print(rxByte, HEX);
#endif
                    if (_RxByteCnt == MAX_KNX_TELEGRAM_SIZE - 2)
                    {
                        println("invalid telegram size");
                        enterRxWaitEOP();
                    }
                    else
                    {
                        buffer[_RxByteCnt++] = rxByte;
                    }

                    if (_RxByteCnt == buffer[6] + 7 + 2)
                    {
                        //complete Frame received, payloadLength+1 for TCPI +1 for CRC
                        //check if crc is correct
                        if (rxByte == (uint8_t)(~_xorSum))
                        {
                            if (!_isEcho)
                            {
                                _receiveBuffer[0] = 0x29;
                                _receiveBuffer[1] = 0;
#ifdef DBG_TRACE
                                unsigned long runTime = millis();
#endif
                                frameBytesReceived(_receiveBuffer, _RxByteCnt + 2);
#ifdef DBG_TRACE
                                runTime = millis() - runTime;
                                if (runTime > (OVERRUN_COUNT*14)/10)
                                {
                                    // complain when the runtime was long than the OVERRUN_COUNT allows
                                    print("processing received frame took: "); print(runTime); println(" ms");
                                }
#endif
                            }
                            _rxState = RX_WAIT_START;
#ifdef DBG_TRACE
                            println("RX_WAIT_START");
#endif
                        }
                        else
                        {
                            println("frame with invalid crc ignored");
                            enterRxWaitEOP();
                        }
                    }
                    else
                    {
                        _xorSum ^= rxByte;
                    }
                    break;
                case RX_WAIT_EOP:
                    if (millis() - _lastByteRxTime > EOP_TIMEOUT)
                    {
                        // found a gap
                        _rxState = RX_WAIT_START;
#ifdef DBG_TRACE
                        println("RX_WAIT_START");
#endif
                        break;
                    }
                    if (_platform.uartAvailable())
                    {
                        _platform.readUart();
                        _lastByteRxTime = millis();
                    }
                    break;
                default:
                    println("invalid _rxState");
                    enterRxWaitEOP();
                    break;
            }
        } while (_rxState == RX_L_ADDR && (stayInRx || _platform.uartAvailable()));

        // Check for spurios DATA_CONN message
        if (dataConnMsg && _txState != TX_WAIT_CONN) {
            println("unexpected L_DATA_CON");
        }

        switch (_txState)
        {
            case TX_IDLE:
                if (!isTxQueueEmpty())
                {
                    loadNextTxFrame();
                    _txState = TX_FRAME;
#ifdef DBG_TRACE
                    println("TX_FRAME");
#endif
                }
                break;
            case TX_FRAME:
                if (millis() - _lastByteTxTime > TX_TIMEPAUSE)
                {
                    if (sendSingleFrameByte() == false)
                    {
                        _waitConfirmStartTime = millis();
                        _txState = TX_WAIT_CONN;
#ifdef DBG_TRACE
                        println("TX_WAIT_CONN");
#endif
                    }
                    else
                    {
                        _lastByteTxTime = millis();
                    }
                }
                break;
            case TX_WAIT_CONN:
                if (dataConnMsg)
                {
                    dataConBytesReceived(_receiveBuffer, _RxByteCnt + 2, (dataConnMsg & SUCCESS));
                    delete[] _sendBuffer;
                    _sendBuffer = 0;
                    _sendBufferLength = 0;
                    _txState = TX_IDLE;
                }
                else if (millis() - _waitConfirmStartTime > CONFIRM_TIMEOUT)
                {
                    println("L_DATA_CON not received within expected time");
                    uint8_t cemiBuffer[MAX_KNX_TELEGRAM_SIZE];
                    cemiBuffer[0] = 0x29;
                    cemiBuffer[1] = 0;
                    memcpy((cemiBuffer + 2), _sendBuffer, _sendBufferLength);
                    dataConBytesReceived(cemiBuffer, _sendBufferLength + 2, false);
                    delete[] _sendBuffer;
                    _sendBuffer = 0;
                    _sendBufferLength = 0;
                    _txState = TX_IDLE;
#ifdef DBG_TRACE
                    println("TX_IDLE");
#endif
                }
                break;
        }
    } while (_platform.uartAvailable());
}

bool TpUartDataLinkLayer::sendFrame(CemiFrame& frame)
{
    if (!_enabled)
    {
        dataConReceived(frame, false);
        return false;
    }

    addFrameTxQueue(frame);
    return true;
}

bool TpUartDataLinkLayer::resetChip()
{
    if(_waitConfirmStartTime > 0) return false;
    uint8_t cmd = U_RESET_REQ;
    _platform.writeUart(cmd);
    
    int resp = _platform.readUart();
    if (resp == U_RESET_IND)
        return true;

    _waitConfirmStartTime = millis();
    return false;
}

bool TpUartDataLinkLayer::resetChipTick()
{
    int resp = _platform.readUart();
    if (resp == U_RESET_IND)
    {
        _waitConfirmStartTime = 0;
        return true;
    }
    else if (millis() - _waitConfirmStartTime > RESET_TIMEOUT)
        _waitConfirmStartTime = 0;
    
    return false;
}

void TpUartDataLinkLayer::stopChip()
{
#ifdef NCN5120
    uint8_t cmd = U_STOP_MODE_REQ;
    _platform.writeUart(cmd);
    while (true)
    {
        int resp = _platform.readUart();
        if (resp == U_STOP_MODE_IND)
            break;
    }
#endif
}

TpUartDataLinkLayer::TpUartDataLinkLayer(DeviceObject& devObj,
                                         NetworkLayerEntity &netLayerEntity,
                                         Platform& platform,
                                         ITpUartCallBacks& cb,
                                         DataLinkLayerCallbacks* dllcb)
    : DataLinkLayer(devObj, netLayerEntity, platform),
      _cb(cb),
      _dllcb(dllcb)
{
}

void TpUartDataLinkLayer::frameBytesReceived(uint8_t* buffer, uint16_t length)
{
    //printHex("=>", buffer, length);
#ifdef KNX_ACTIVITYCALLBACK
    if(_dllcb)
        _dllcb->activity((_netIndex << KNX_ACTIVITYCALLBACK_NET) | (KNX_ACTIVITYCALLBACK_DIR_RECV << KNX_ACTIVITYCALLBACK_DIR));
#endif
    CemiFrame frame(buffer, length);
    frameReceived(frame);
}

void TpUartDataLinkLayer::dataConBytesReceived(uint8_t* buffer, uint16_t length, bool success)
{
    //printHex("=>", buffer, length);
    CemiFrame frame(buffer, length);
    dataConReceived(frame, success);
}

void TpUartDataLinkLayer::enabled(bool value)
{
    if (value && !_enabled)
    {
        _platform.setupUart();

        uint8_t cmd = U_RESET_REQ;
        _platform.writeUart(cmd);
        _waitConfirmStartTime = millis();
        bool flag = false;

        while (true)
        {
            int resp = _platform.readUart();
            if (resp == U_RESET_IND)
            {
                flag = true;
                break;
            }
            else if (millis() - _waitConfirmStartTime > RESET_TIMEOUT)
            {
                flag = false;
                break;
            }
        }

        if (flag)
        {
            _enabled = true;
            print("ownaddr ");
            println(_deviceObject.individualAddress(), HEX);
        }
        else
        {
            _enabled = false;
            println("ERROR, TPUART not responding");
        }
        return;
    }

    if (!value && _enabled)
    {
        _enabled = false;
        stopChip();
        _platform.closeUart();
        return;
    }
}

bool TpUartDataLinkLayer::enabled() const
{
    return _enabled;
}

DptMedium TpUartDataLinkLayer::mediumType() const
{
    return DptMedium::KNX_TP1;
}

bool TpUartDataLinkLayer::sendSingleFrameByte()
{
    uint8_t cmd[2];

    uint8_t idx = _TxByteCnt >> 6;

    if (_sendBuffer == NULL)
        return false;

    if (_TxByteCnt < _sendBufferLength)
    {
        if (idx != _oldIdx)
        {
            _oldIdx = idx;
            cmd[0] = U_L_DATA_OFFSET_REQ | idx;
            _platform.writeUart(cmd, 1);
        }

        if (_TxByteCnt != _sendBufferLength - 1)
            cmd[0] = U_L_DATA_START_CONT_REQ | (_TxByteCnt & 0x3F);
        else
            cmd[0] = U_L_DATA_END_REQ | (_TxByteCnt & 0x3F);

        cmd[1] = _sendBuffer[_TxByteCnt];
#ifdef DBG_TRACE
        print(cmd[1], HEX);
#endif

        _platform.writeUart(cmd, 2);
        _TxByteCnt++;
    }
    
    // Check for last byte send
    if (_TxByteCnt >= _sendBufferLength)
    {
        _TxByteCnt = 0;
#ifdef KNX_ACTIVITYCALLBACK
        if(_dllcb)
            _dllcb->activity((_netIndex << KNX_ACTIVITYCALLBACK_NET) | (KNX_ACTIVITYCALLBACK_DIR_SEND << KNX_ACTIVITYCALLBACK_DIR));
#endif
        return false;
    }
    return true;
}

void TpUartDataLinkLayer::addFrameTxQueue(CemiFrame& frame)
{
    _tx_queue_frame_t* tx_frame = new _tx_queue_frame_t;
    tx_frame->length = frame.telegramLengthtTP();
    tx_frame->data = new uint8_t[tx_frame->length];
    tx_frame->next = NULL;
    frame.fillTelegramTP(tx_frame->data);

    if (_tx_queue.back == NULL)
    {
        _tx_queue.front = _tx_queue.back = tx_frame;
    }
    else
    {
        _tx_queue.back->next = tx_frame;
        _tx_queue.back = tx_frame;
    }
}

bool TpUartDataLinkLayer::isTxQueueEmpty()
{
    if (_tx_queue.front == NULL)
    {
        return true;
    }
    return false;
}

void TpUartDataLinkLayer::loadNextTxFrame()
{
    if (_tx_queue.front == NULL)
    {
        return;
    }
    _tx_queue_frame_t* tx_frame = _tx_queue.front;
    _sendBuffer = tx_frame->data;
    _sendBufferLength = tx_frame->length;
    _tx_queue.front = tx_frame->next;

    if (_tx_queue.front == NULL)
    {
        _tx_queue.back = NULL;
    }
    delete tx_frame;
}
#endif
