#include "tpuart_data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"
#include "cemi_frame.h"

#include <stdio.h>
#include <string.h>

#define SerialKNX Serial1

// NCN5120

// services Host -> Controller :
// internal commands, device specific
#define U_RESET_REQ          0x01
#define U_STATE_REQ          0x02
#define U_SET_BUSY_REQ       0x03
#define U_QUIT_BUSY_REQ      0x04
#define U_BUSMON_REQ         0x05
#define U_SET_ADDRESS_REQ    0xF1
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
#define U_L_DATA_END_REQ        0x47 //-0x7F

//serices to host controller

// DLL services (device is transparent)
#define L_DATA_STANDARD_IND 0x81
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

void resetChip()
{
    uint8_t cmd = U_RESET_REQ;
    SerialKNX.write(cmd);
    while (true)
    {
        int resp = SerialKNX.read();
        if (resp == U_RESET_IND)
            break;
    }
}

void stopChip()
{
    uint8_t cmd = U_STOP_MODE_REQ;
    SerialKNX.write(cmd);
    while (true)
    {
        int resp = SerialKNX.read();
        if (resp == U_STOP_MODE_IND)
            break;
    }
}

void setAddress(uint16_t addr)
{
    if (addr == 0)
        return;

    uint8_t cmd[4];
    cmd[1] = (uint8_t)(addr >> 8);
    cmd[2] = (uint8_t)(addr & 0xff);
    //cmd[3] is don't care

    SerialKNX.write(cmd, 4);
    while (true)
    {
        uint8_t resp = SerialKNX.read();
        resp &= U_CONFIGURE_MASK;
        if (resp == U_CONFIGURE_IND)
            break;
    }
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

    if (SerialKNX.available() == 0)
        return;

    uint8_t firstByte = SerialKNX.read();

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
    uint8_t tmp = firstByte & L_DATA_MASK;
    if (tmp != L_DATA_STANDARD_IND && tmp != L_DATA_EXTENDED_IND)
        return false;

    uint8_t buffer[512];
    int len = 0;
    // TODO: get rest of data, compare to sendbuffer 
    CemiFrame frame(buffer, len);
    frameRecieved(frame);
    return true;
}

bool TpUartDataLinkLayer::checkDataCon(uint8_t firstByte)
{
    uint8_t tmp = firstByte & L_DATA_CON_MASK;
    if (tmp != L_DATA_CON)
        return false;

    if (_sendBuffer == 0)
    {
        SerialDBG.println("got unexpected L_DATA_CON");
        return true;
    }

    _sendResult = (firstByte & SUCCESS) > 0;
    _sendBuffer = 0;

    return true;
}

bool TpUartDataLinkLayer::checkPollDataInd(uint8_t firstByte)
{
    if (firstByte != L_POLL_DATA_IND)
        return false;

    // not sure if this can happen
    SerialDBG.println("got L_POLL_DATA_IND");
    return true;
}

bool TpUartDataLinkLayer::checkAckNackInd(uint8_t firstByte)
{
    uint8_t tmp = firstByte & L_ACKN_MASK;
    if (tmp != L_ACKN_IND)
        return false;

    // this can only happen in bus monitor mode
    SerialDBG.println("got L_ACKN_IND");
    return true;
}

bool TpUartDataLinkLayer::checkResetInd(uint8_t firstByte)
{
    if (firstByte != U_RESET_IND)
        return false;

    SerialDBG.println("got U_RESET_IND");
    return true;
}

bool TpUartDataLinkLayer::checkStateInd(uint8_t firstByte)
{
    uint8_t tmp = firstByte & U_STATE_IND;
    if (tmp != U_STATE_IND)
        return false;

    SerialDBG.print("got U_STATE_IND: 0x");
    SerialDBG.print(firstByte, HEX);
    SerialDBG.println();
    return true;
}

bool TpUartDataLinkLayer::checkFrameStateInd(uint8_t firstByte)
{
    uint8_t tmp = firstByte & U_FRAME_STATE_MASK;
    if (tmp != U_FRAME_STATE_IND)
        return false;

    SerialDBG.print("got U_FRAME_STATE_IND: 0x");
    SerialDBG.print(firstByte, HEX);
    SerialDBG.println();
    return true;
}

bool TpUartDataLinkLayer::checkConfigureInd(uint8_t firstByte)
{
    uint8_t tmp = firstByte & U_CONFIGURE_MASK;
    if (tmp != U_CONFIGURE_IND)
        return false;

    SerialDBG.print("got U_CONFIGURE_IND: 0x");
    SerialDBG.print(firstByte, HEX);
    SerialDBG.println();
    return true;
}

bool TpUartDataLinkLayer::checkFrameEndInd(uint8_t firstByte)
{
    if (firstByte != U_FRAME_END_IND)
        return false;

    SerialDBG.println("got U_FRAME_END_IND");
    return true;
}

bool TpUartDataLinkLayer::checkStopModeInd(uint8_t firstByte)
{
    if (firstByte != U_STOP_MODE_IND)
        return false;

    SerialDBG.println("got U_STOP_MODE_IND");
    return true;
}

bool TpUartDataLinkLayer::checkSystemStatInd(uint8_t firstByte)
{
    if (firstByte != U_SYSTEM_STAT_IND)
        return false;

    SerialDBG.print("got U_SYSTEM_STAT_IND: 0x");
    while (true)
    {
        int tmp = SerialKNX.read();
        if (tmp < 0)
            continue;

        SerialDBG.print(tmp, HEX);
        break;
    }
    SerialDBG.println();
    return true;
}

void TpUartDataLinkLayer::handleUnexpected(uint8_t firstByte)
{
    SerialDBG.print("got UNEXPECTED: 0x");
    SerialDBG.print(firstByte, HEX);
    SerialDBG.println();
}

void TpUartDataLinkLayer::enabled(bool value)
{
    if (value && !_enabled)
    {
        SerialKNX.begin(19200, SERIAL_8E1);
        while (!SerialKNX)
            ;

        resetChip();
        setAddress(_deviceObject.induvidualAddress());
        _enabled = true;
        return;
    }

    if (!value && _enabled)
    {
        _enabled = false;
        stopChip();
        SerialKNX.end();
        return;
    }
}

bool TpUartDataLinkLayer::enabled() const
{
    return _enabled;
}


void TpUartDataLinkLayer::sendBytes(uint8_t* bytes, uint16_t length)
{
    uint8_t cmd[2];

    for (int i = 0; i < length; i++)
    {
        if (i != length - 1)
            cmd[0] = U_L_DATA_START_CONT_REQ | i;
        else
            cmd[0] = U_L_DATA_END_REQ;

        cmd[1] = bytes[i];

        SerialKNX.write(cmd, 2);
    }
}