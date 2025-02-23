#include "tpdu.h"

#include "../bits.h"
#include "../datalink_layer/cemi_frame.h"

namespace Knx
{
    TPDU::TPDU(uint8_t* data, CemiFrame& frame)
        : _data(data), _frame(frame)
    {
    }

    TpduType TPDU::type() const
    {
        if (control())
        {
            if (numbered())
            {
                if ((_data[0] & 1) == 0)
                    return Ack;
                else
                    return Nack;
            }
            else if ((_data[0] & 1) == 0)
                return Connect;
            else
                return Disconnect;
        }
        else
        {
            if (_frame.addressType() == GroupAddress)
            {
                if (_frame.destinationAddress() == 0)
                    return DataBroadcast;
                else
                    return DataGroup;
            }
            else if (numbered())
                return DataConnected;
            else
                return DataInduvidual;
        }
    }

    void TPDU::type(TpduType type)
    {
        switch (type)
        {
            case DataBroadcast:
            case DataGroup:
            case DataInduvidual:
                _data[0] &= 0x3;
                break;

            case DataConnected:
                _data[0] &= 0xC3;
                _data[0] |= 0x40;
                break;

            case Connect:
                _data[0] = 0x80;
                break;

            case Disconnect:
                _data[0] = 0x81;
                break;

            case Ack:
                _data[0] &= ~0xC3;
                _data[0] |= 0xC2;
                break;

            case Nack:
                _data[0] |= 0xC3;
                break;
        }
    }

    bool TPDU::numbered() const
    {
        return (_data[0] & 0x40) > 0;
    }

    void TPDU::numbered(bool value)
    {
        if (value)
            _data[0] |= 0x40;
        else
            _data[0] &= ~0x40;
    }

    bool TPDU::control() const
    {
        return (_data[0] & 0x80) > 0;
    }

    void TPDU::control(bool value)
    {
        if (value)
            _data[0] |= 0x80;
        else
            _data[0] &= ~0x80;
    }

    uint8_t TPDU::sequenceNumber() const
    {
        return ((_data[0] >> 2) & 0xF);
    }

    void TPDU::sequenceNumber(uint8_t value)
    {
        _data[0] &= ~(0xF << 2);
        _data[0] |= ((value & 0xF) << 2);
    }

    APDU& TPDU::apdu()
    {
        return _frame.apdu();
    }

    CemiFrame& TPDU::frame()
    {
        return _frame;
    }

    void TPDU::printIt() const
    {
#ifndef KNX_NO_PRINT
        print("TPDU: ");
        print(enum_name(type()));
        print(" ");

        if (control())
            print("control ");

        if (numbered())
        {
            print("numbered sequence: ");
            print(sequenceNumber());
        }

#endif
    }
} // namespace Knx