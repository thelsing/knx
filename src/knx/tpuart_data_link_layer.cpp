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

//loop states
#define IDLE                0
#define RX_FIRST_BYTE       1
#define RX_L_DATA           2
#define RX_WAIT_DATA_CON    3
#define TX_FRAME            4

#define BYTE_TIMEOUT          10   //milli seconds
#define CONFIRM_TIMEOUT       500  //milli seconds
#define RESET_TIMEOUT         100 //milli seconds

void TpUartDataLinkLayer::loop()
{

    _receiveBuffer[0] = 0x29;
    _receiveBuffer[1] = 0;
    uint8_t* buffer = _receiveBuffer + 2;
    uint8_t rxByte;

    if (!_enabled)
    {
        if (millis() - _lastResetChipTime > 1000)
        { 
            //reset chip every 1 seconds
            _lastResetChipTime = millis();
            _enabled = resetChip();
        }
    }

    if (!_enabled)
        return;

    switch (_loopState)
    {
        case IDLE:
            if (_platform.uartAvailable())
            {
                _loopState = RX_FIRST_BYTE;
            }
            else
            {
                if (!_waitConfirm && !isTxQueueEmpty())
                {
                    loadNextTxFrame();
                    _loopState = TX_FRAME;
                }
            }
            break;
        case TX_FRAME:
            if (sendSingleFrameByte() == false)
            {
                _waitConfirm = true;
                _waitConfirmStartTime = millis();
                _loopState = IDLE;
            }
            break;
        case RX_FIRST_BYTE:
            rxByte = _platform.readUart();
            _lastByteRxTime = millis();
            _RxByteCnt = 0;
            _xorSum = 0;
            if ((rxByte & L_DATA_MASK) == L_DATA_STANDARD_IND)
            {
                buffer[_RxByteCnt++] = rxByte;
                _xorSum ^= rxByte;
                _RxByteCnt++; //convert to L_DATA_EXTENDED
                _convert = true;
                _loopState = RX_L_DATA;
                break;
            }
            else if ((rxByte & L_DATA_MASK) == L_DATA_EXTENDED_IND)
            {
                buffer[_RxByteCnt++] = rxByte;
                _xorSum ^= rxByte;
                _convert = false;
                _loopState = RX_L_DATA;
                break;
            }
            else if ((rxByte & L_DATA_CON_MASK) == L_DATA_CON)
            {
                println("got unexpected L_DATA_CON");
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
                print("got U_STATE_IND: 0x");
                print(rxByte, HEX);
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
            _loopState = IDLE;
            break;
        case RX_L_DATA:
            if (millis() - _lastByteRxTime > BYTE_TIMEOUT)
            {
                _RxByteCnt = 0;
                _loopState = IDLE;
                println("Timeout during RX_L_DATA");
                break;
            }
            if (!_platform.uartAvailable())
                break;
            _lastByteRxTime = millis();
            rxByte = _platform.readUart();

            if (_RxByteCnt == MAX_KNX_TELEGRAM_SIZE)
            {
                _loopState = IDLE;
                println("invalid telegram size");
            }
            else
            {
                buffer[_RxByteCnt++] = rxByte;
            }

            if (_RxByteCnt == 7)
            {
                //Destination Address + payload available
                _xorSum ^= rxByte;
                //check if echo
                if (_sendBuffer != nullptr && (!((buffer[0] ^ _sendBuffer[0]) & ~0x20) && !memcmp(buffer + _convert + 1, _sendBuffer + 1, 5)))
                { //ignore repeated bit of control byte
                    _isEcho = true;
                }
                else
                {
                    _isEcho = false;
                }

                //convert into Extended.ind
                if (_convert)
                {
                    uint8_t payloadLength = buffer[6] & 0x0F;
                    buffer[1] = buffer[6] & 0xF0;
                    buffer[6] = payloadLength;
                }

                if (!_isEcho)
                {
                    uint8_t c = 0x10;

                    // The bau knows everything and could either check the address table object (normal device)
                    // or any filter tables (coupler) to see if we are addressed.

                    //check if individual or group address
                    bool isGroupAddress = (buffer[1] & 0x80) != 0;
                    uint16_t addr = getWord(buffer + 4);

                    if (_cb.isAckRequired(addr, isGroupAddress))
                    {
                        c |= 0x01;
                    }

                    _platform.writeUart(c);
                }
            }
            else if (_RxByteCnt == buffer[6] + 7 + 2)
            {
                //complete Frame received, payloadLength+1 for TCPI +1 for CRC
                if (rxByte == (uint8_t)(~_xorSum))
                {
                    //check if crc is correct
                    if (_isEcho && _sendBuffer != NULL)
                    {
                        //check if it is realy an echo, rx_crc = tx_crc
                        if (rxByte == _sendBuffer[_sendBufferLength - 1])
                            _isEcho = true;
                        else
                            _isEcho = false;
                    }
                    if (_isEcho)
                    {
                        _loopState = RX_WAIT_DATA_CON;
                    }
                    else
                    {
                        frameBytesReceived(_receiveBuffer, _RxByteCnt + 2);
                        _loopState = IDLE;
                    }
                }
                else
                {
                    println("frame with invalid crc ignored");
                    _loopState = IDLE;
                }
            }
            else
            {
                _xorSum ^= rxByte;
            }
            break;
        case RX_WAIT_DATA_CON:
            if (!_platform.uartAvailable())
                break;
            rxByte = _platform.readUart();
            _lastByteRxTime = millis();
            if ((rxByte & L_DATA_CON_MASK) == L_DATA_CON)
            {
                //println("L_DATA_CON received");
                dataConBytesReceived(_receiveBuffer, _RxByteCnt + 2, ((rxByte & SUCCESS) > 0));
                _waitConfirm = false;
                delete[] _sendBuffer;
                _sendBuffer = 0;
                _sendBufferLength = 0;
                _loopState = IDLE;
            }
            else
            {
                //should not happen
                println("expected L_DATA_CON not received");
                dataConBytesReceived(_receiveBuffer, _RxByteCnt + 2, false);
                _waitConfirm = false;
                delete[] _sendBuffer;
                _sendBuffer = 0;
                _sendBufferLength = 0;
                _loopState = IDLE;
            }
            break;
        default:
            break;
    }

    if (_waitConfirm)
    {
        if (millis() - _waitConfirmStartTime > CONFIRM_TIMEOUT)
        {
            println("L_DATA_CON not received within expected time");
            uint8_t cemiBuffer[MAX_KNX_TELEGRAM_SIZE];
            cemiBuffer[0] = 0x29;
            cemiBuffer[1] = 0;
            memcpy((cemiBuffer + 2), _sendBuffer, _sendBufferLength);
            dataConBytesReceived(cemiBuffer, _sendBufferLength + 2, false);
            _waitConfirm = false;
            delete[] _sendBuffer;
            _sendBuffer = 0;
            _sendBufferLength = 0;
            if (_loopState == RX_WAIT_DATA_CON)
                _loopState = IDLE;
        }
    }
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
    uint8_t cmd = U_RESET_REQ;
    _platform.writeUart(cmd);
    _waitConfirmStartTime = millis();
    while (true)
    {
        int resp = _platform.readUart();
        if (resp == U_RESET_IND)
            return true;
        else if (millis() - _waitConfirmStartTime > RESET_TIMEOUT)
            return false;
    }
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
                                         ITpUartCallBacks& cb)
    : DataLinkLayer(devObj, netLayerEntity, platform),
      _cb(cb)
{
}

void TpUartDataLinkLayer::frameBytesReceived(uint8_t* buffer, uint16_t length)
{
    //printHex("=>", buffer, length);
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

        if (resetChip())
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
    uint8_t idx = _TxByteCnt / 64;

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
            cmd[0] = U_L_DATA_START_CONT_REQ | _TxByteCnt;
        else
            cmd[0] = U_L_DATA_END_REQ | _TxByteCnt;

        cmd[1] = _sendBuffer[_TxByteCnt];

        _platform.writeUart(cmd, 2);
        _TxByteCnt++;
        return true;
    }
    else
    {
        _TxByteCnt = 0;
        return false;
    }
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
