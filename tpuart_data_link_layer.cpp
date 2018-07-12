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
#define U_L_DATA_START_REQ   0x80
#define U_L_DATA_CONT_REQ    0x81 //-0xBF
#define U_L_DATA_END_REQ     0x47 //-0x7F

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
        uint8_t resp = SerialKNX.read();
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
        uint8_t resp = SerialKNX.read();
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
    uint16_t length = frame.telegramLengthtTP();
    uint8_t* buffer = new uint8_t[length];
    frame.fillTelegramTP(buffer);
    
    bool success = sendBytes(buffer, length);

    delete[] buffer;
    return success;
}

void TpUartDataLinkLayer::loop()
{
    if (!_enabled)
        return;

    uint8_t buffer[512];
    int len = 0;
    // TODO: implement receiving of frames

    CemiFrame frame(buffer, len);
    frameRecieved(frame);
}

void TpUartDataLinkLayer::enabled(bool value)
{
    if (value && !_enabled)
    {
        SerialKNX.begin(19200, SERIAL_8E1);
        resetChip();
        setAddress(_deviceObject.induvidualAddress());
        _enabled = true;
        return;
    }

    if(!value && _enabled)
    {
        stopChip();
        _enabled = false;
        return;
    }
}

bool TpUartDataLinkLayer::enabled() const
{
    return _enabled;
}


bool TpUartDataLinkLayer::sendBytes(uint8_t* bytes, uint16_t length)
{
    if (!_enabled)
        return false;

    //TODO: implement
    return false;
}