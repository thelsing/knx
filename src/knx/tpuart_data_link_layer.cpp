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
#define U_RESET_IND         0x03
#define U_STATE_IND         0x07
#define SLAVE_COLLISION     0x80
#define RECEIVE_ERROR       0x40
#define TRANSMIT_ERROR      0x20
#define PROTOCOL_ERROR      0x10
#define TEMPERATURE_WARNING 0x08
#define U_FRAME_STATE_IND   0x13
#define U_FRAME_STATE_MASK  0x17
#define PARITY_BIT_ERROR    0x80
#define CHECKSUM_LENGTH_ERROR 0x40
#define TIMING_ERROR        0x20
#define U_CONFIGURE_IND     0x01
#define U_CONFIGURE_MASK    0x83
#define AUTO_ACKNOWLEDGE    0x20
#define AUTO_POLLING        0x10
#define CRC_CCITT           0x80
#define FRAME_END_WITH_MARKER 0x40
#define U_FRAME_END_IND     0xCB
#define U_STOP_MODE_IND     0x2B
#define U_SYSTEM_STAT_IND   0x4B

void TpUartDataLinkLayer::resetChip()
{
    uint8_t cmd = U_RESET_REQ;
    _platform.writeUart(cmd);
    while (true)
    {
        int resp = _platform.readUart();
        if (resp == U_RESET_IND)
            break;
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


TpUartDataLinkLayer::TpUartDataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab,
    NetworkLayer& layer, Platform& platform) : DataLinkLayer(devObj, addrTab, layer, platform)
{
}


bool TpUartDataLinkLayer::sendFrame(CemiFrame& frame)
{
    if (!_enabled)
        return false;

    uint16_t length = frame.telegramLengthtTP();
    uint8_t* buffer = new uint8_t[length];
    frame.fillTelegramTP(buffer);

    _sendBuffer = buffer;
    _sendResult = false;
    _sendBufferLength = length;
    sendBytes(buffer, length);

    while (_sendBuffer != 0)
        loop();

    delete[] buffer;
    return _sendResult;
}

void TpUartDataLinkLayer::loop()
{
    if (!_enabled)
        return;

    if (_platform.uartAvailable() == 0)
        return;

    uint8_t firstByte = _platform.readUart();

    if (checkDataInd(firstByte))
        return;

    if (checkDataCon(firstByte))
        return;

    if (checkPollDataInd(firstByte))
        return;

    if (checkAckNackInd(firstByte))
        return;

    if (checkResetInd(firstByte))
        return;

    if (checkStateInd(firstByte))
        return;

    if (checkFrameStateInd(firstByte))
        return;

    if (checkConfigureInd(firstByte))
        return;

    if (checkFrameEndInd(firstByte))
        return;

    if (checkStopModeInd(firstByte))
        return;

    if (checkSystemStatInd(firstByte))
        return;

    handleUnexpected(firstByte);
}

bool TpUartDataLinkLayer::checkDataInd(uint8_t firstByte)
{
    const size_t bufferSize = 512;

    uint8_t tmp = firstByte & L_DATA_MASK;
    if (tmp != L_DATA_STANDARD_IND && tmp != L_DATA_EXTENDED_IND)
        return false;

    int len = 0;
    uint8_t cemiBuffer[bufferSize + 2];
    cemiBuffer[0] = 0x29;
    cemiBuffer[1] = 0;
    uint8_t* buffer = cemiBuffer + 2;
    buffer[0] = firstByte;

    uint8_t payloadLength = 0;
    if (tmp == L_DATA_STANDARD_IND)
    {
        //convert to extended frame format
        _platform.readBytesUart(buffer + 2, 5);
        sendAck((AddressType)(buffer[6] & GroupAddress), getWord(buffer + 4));
        payloadLength = buffer[6] & 0xF;
        _platform.readBytesUart(buffer + 7, payloadLength + 2); //+1 for TCPI +1 for CRC
        buffer[1] = buffer[6] & 0xF0;
        buffer[6] = payloadLength;
    }
    else
    {
        //extended frame
        _platform.readBytesUart(buffer + 1, 6);
        sendAck((AddressType)(buffer[1] & GroupAddress), getWord(buffer + 4));
        payloadLength = buffer[6];
        _platform.readBytesUart(buffer + 7, payloadLength + 2); //+1 for TCPI +1 for CRC
    }
    len = payloadLength + 9;
    
    
    const uint8_t queueLength = 5;
    static uint8_t buffers[queueLength][bufferSize + 2];
    static uint16_t bufferLengths[queueLength];

    if (_sendBuffer != 0)
    {
        // we are trying to send a telegram queue received telegrams until we get our sent telegram

        if (len == _sendBufferLength && memcmp(_sendBuffer, buffer, len) == 0)
        {
            //we got the send telegramm back next byte is L_Data.con byte
            uint8_t confirm = _platform.readUart();
            confirm &= L_DATA_CON_MASK;
            _sendResult = (confirm > 0);
            _sendBuffer = 0;
            _sendBufferLength = 0;

            return true;
        }

        // queue telegram, if we get more the queueLength before send succeeds 
        // ignore the telegram
        for (int i = 0; i < queueLength; i++)
        {
            if (bufferLengths[i] != 0)
                continue;

            bufferLengths[i] = len;
            memcpy(&buffers[i][0], cemiBuffer, len + 2);
            break;
        }
    }
    else
    {
        // process all previously queued telegramms first
        for (int i = 0; i < queueLength; i++)
        {
            if (bufferLengths[i] == 0)
                break;

            frameBytesReceived(&buffers[i][0], bufferLengths[i]+2);
            bufferLengths[i] = 0;
        }

        frameBytesReceived(cemiBuffer, len+2);
    }

    return true;
}

void TpUartDataLinkLayer::frameBytesReceived(uint8_t* buffer, uint16_t length)
{
    //printHex("=>", buffer, length);
    CemiFrame frame(buffer, length);

    frameRecieved(frame);
}

bool TpUartDataLinkLayer::checkDataCon(uint8_t firstByte)
{
    uint8_t tmp = firstByte & L_DATA_CON_MASK;
    if (tmp != L_DATA_CON)
        return false;

    if (_sendBuffer == 0)
    {
        println("got unexpected L_DATA_CON");
        return true;
    }

    _sendResult = (firstByte & SUCCESS) > 0;
    _sendBuffer = 0;
    _sendBufferLength = 0;

    return true;
}

bool TpUartDataLinkLayer::checkPollDataInd(uint8_t firstByte)
{
    if (firstByte != L_POLL_DATA_IND)
        return false;

    // not sure if this can happen
    println("got L_POLL_DATA_IND");
    return true;
}

bool TpUartDataLinkLayer::checkAckNackInd(uint8_t firstByte)
{
    uint8_t tmp = firstByte & L_ACKN_MASK;
    if (tmp != L_ACKN_IND)
        return false;

    // this can only happen in bus monitor mode
    println("got L_ACKN_IND");
    return true;
}

bool TpUartDataLinkLayer::checkResetInd(uint8_t firstByte)
{
    if (firstByte != U_RESET_IND)
        return false;

    println("got U_RESET_IND");
    return true;
}

bool TpUartDataLinkLayer::checkStateInd(uint8_t firstByte)
{
    uint8_t tmp = firstByte & U_STATE_IND;
    if (tmp != U_STATE_IND)
        return false;

    print("got U_STATE_IND: 0x");
    print(firstByte, HEX);
    println();
    return true;
}

bool TpUartDataLinkLayer::checkFrameStateInd(uint8_t firstByte)
{
    uint8_t tmp = firstByte & U_FRAME_STATE_MASK;
    if (tmp != U_FRAME_STATE_IND)
        return false;

    print("got U_FRAME_STATE_IND: 0x");
    print(firstByte, HEX);
    println();
    return true;
}

bool TpUartDataLinkLayer::checkConfigureInd(uint8_t firstByte)
{
    uint8_t tmp = firstByte & U_CONFIGURE_MASK;
    if (tmp != U_CONFIGURE_IND)
        return false;

    print("got U_CONFIGURE_IND: 0x");
    print(firstByte, HEX);
    println();
    return true;
}

bool TpUartDataLinkLayer::checkFrameEndInd(uint8_t firstByte)
{
    if (firstByte != U_FRAME_END_IND)
        return false;

    println("got U_FRAME_END_IND");
    return true;
}

bool TpUartDataLinkLayer::checkStopModeInd(uint8_t firstByte)
{
    if (firstByte != U_STOP_MODE_IND)
        return false;

    println("got U_STOP_MODE_IND");
    return true;
}

bool TpUartDataLinkLayer::checkSystemStatInd(uint8_t firstByte)
{
    if (firstByte != U_SYSTEM_STAT_IND)
        return false;

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
    return true;
}

void TpUartDataLinkLayer::handleUnexpected(uint8_t firstByte)
{
    print("got UNEXPECTED: 0x");
    print(firstByte, HEX);
    println();
}

void TpUartDataLinkLayer::enabled(bool value)
{
    if (value && !_enabled)
    {
        _platform.setupUart();
        print("ownaddr ");
        println(_deviceObject.induvidualAddress(), HEX);
        resetChip();
        _enabled = true;
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


void TpUartDataLinkLayer::sendBytes(uint8_t* bytes, uint16_t length)
{
    //printHex("<=", bytes, length);
    uint8_t cmd[2];

    uint8_t oldIdx = 0;
        
    for (int i = 0; i < length; i++)
    {
        uint8_t idx = length / 64;
        if (idx != oldIdx)
        {
            oldIdx = idx;
            cmd[0] = U_L_DATA_OFFSET_REQ | idx;
            _platform.writeUart(cmd, 1);
        }

        if (i != length - 1)
            cmd[0] = U_L_DATA_START_CONT_REQ | i;
        else
            cmd[0] = U_L_DATA_END_REQ | i;

        cmd[1] = bytes[i];

        _platform.writeUart(cmd, 2);
    }
}

void TpUartDataLinkLayer::sendAck(AddressType type, uint16_t address)
{
    if ( (type == InduvidualAddress &&  _deviceObject.induvidualAddress() == address) 
      || (type == GroupAddress &&  (_groupAddressTable.contains(address) || address == 0)))
    {
        //send ack. 
        _platform.writeUart(U_ACK_REQ + 1);
    }
    else
    {
        // send not addressed
        _platform.writeUart(U_ACK_REQ);
    }
}
