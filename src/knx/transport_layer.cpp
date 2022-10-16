#include "transport_layer.h"
#include "apdu.h"
#include "cemi_frame.h"
#include "network_layer.h"
#include "application_layer.h"
#include "platform.h"
#include "bits.h"
#include <stdio.h>

TransportLayer::TransportLayer(ApplicationLayer& layer): _savedFrame(0),
    _savedFrameConnecting(0), _applicationLayer(layer)
{
    _currentState = Closed;
}

void TransportLayer::networkLayer(NetworkLayer& layer)
{
    _networkLayer = &layer;
}

void TransportLayer::groupAddressTable(AddressTableObject &addrTable)
{
    _groupAddressTable = &addrTable;
}

void TransportLayer::dataIndividualIndication(uint16_t destination, HopCountType hopType, Priority priority, uint16_t source, TPDU& tpdu)
{
    //if (tpdu.apdu().length() > 0)
    //{
    //    print.print("<- TL  ");
    //    tpdu.printPDU();
    //    print.print("<- TL  ");
    //    tpdu.apdu().printPDU();
    //}

    uint8_t sequenceNo = tpdu.sequenceNumber();
    switch (tpdu.type())
    {
    case DataInduvidual:
        _applicationLayer.dataIndividualIndication(hopType, priority, source, tpdu.apdu());
        return;
    case DataConnected:
        if (source == _connectionAddress)
        {
            if (sequenceNo == _seqNoRecv)
            {
                //E4
                switch (_currentState)
                {
                case Closed:
                    //A0 nothing
                    break;
                case OpenIdle:
                case OpenWait:
                    A2(source, priority, tpdu.apdu());
                    break;
                case Connecting:
                    _currentState = Closed;
                    A6(destination);
                    break;
                }
            }
            else if(sequenceNo == ((_seqNoRecv -1) & 0xF))
            {
                //E5
                switch (_currentState)
                {
                case Closed:
                    //A0
                    break;
                case OpenIdle:
                case OpenWait:
                case Connecting:
                    A3(source, priority, tpdu);
                    break;
                }
            }
            else
            {
                //E6
                switch (_currentState)
                {
                case Closed:
                    //A0
                    break;
                case OpenIdle:
                case OpenWait:
                    A4(source, priority, tpdu);
                    break;
                case Connecting:
                    A6(destination);
                    break;
                }
            }
        }
        else
        {
            //E7
            switch (_currentState)
            {
            case Closed:
            case OpenIdle:
            case OpenWait:
                //A0
                break;
            case Connecting:
                A10(source);
                break;
            }
        }
        break;
    case Connect:
        if (source == _connectionAddress)
        {
            //E0
            switch (_currentState)
            {
            case Closed:
                _currentState = OpenIdle;
                A1(source);
                break;
            case OpenWait:
            case OpenIdle:
            case Connecting:
                //A0: do nothing
                break;
            }
        }
        else
        {
            //E1
            switch (_currentState)
            {
            case Closed:
                _currentState = OpenIdle;
                A1(source);
                break;
            case OpenIdle:
            case OpenWait:
            case Connecting:
                A10(source);
                break;
            }
        }
        break;
    case Disconnect:
        if (source == _connectionAddress)
        {
            //E2
            switch (_currentState)
            {
            case Closed:
                //A0 do nothing
                break;
            case OpenIdle:
            case OpenWait:
            case Connecting:
                _currentState = Closed;
                A5(source);
                break;
            default:
                break;
            }
        }
        else
        {
            //E3
            //A0: do nothing
        }
        break;
    case Ack:
        if (source == _connectionAddress)
        {
            if (sequenceNo == _seqNoSend)
            {
                //E8
                switch (_currentState)
                {
                case Closed:
                case OpenIdle:
                    //A0
                    break;
                case OpenWait:
                    _currentState = OpenIdle;
                    A8();
                    break;
                case Connecting:
                    _currentState = Closed;
                    A6(source);
                    break;
                }
            }
            else
            {
                //E9
                switch (_currentState)
                {
                case Closed:
                case OpenIdle:
                    //A0
                    break;
                case OpenWait:
                case Connecting:
                    _currentState = Closed;
                    A6(source);
                    break;
                 }
            }
        }
        else
        {
            //E10
            switch (_currentState)
            {
            case Connecting:
                A10(source);
                break;
            default: /* do nothing */
                break;
            }
        }
        break;
    case Nack:
        if (source == _connectionAddress)
        {
            if (sequenceNo != _seqNoSend)
            {
                //E11
                switch (_currentState)
                {
                case Closed:
                case OpenIdle:
                case OpenWait:
                    //A0
                    break;
                case Connecting:
                    _currentState = Closed;
                    A6(source);
                    break;
                }
            }
            else
            {
                if (_repCount < _maxRepCount)
                {
                    //E12
                    switch (_currentState)
                    {
                    case Closed:
                        //A0
                        break;
                    case Connecting:
                    case OpenIdle:
                        _currentState = Closed;
                        A6(source);
                        break;
                    case OpenWait:
                        A9();
                        break;
                    }
                }
                else
                {
                    //E13
                    switch (_currentState)
                    {
                    case Closed:
                        //A0
                        break;
                    case OpenIdle:
                    case OpenWait:
                    case Connecting:
                        _currentState = Closed;
                        A6(source);
                        break;
                    }
                }
            }
        }
        else
        {
            //E14
            switch (_currentState)
            {
            case Closed:
            case OpenIdle:
            case OpenWait:
                //A0
                break;
            case Connecting:
                A10(source);
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
}

void TransportLayer::dataIndividualConfirm(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu, bool status)
{
    TpduType type = tpdu.type();
    switch (type)
    {
    case DataInduvidual:
        _applicationLayer.dataIndividualConfirm(ack, hopType, priority, destination, tpdu.apdu(), status);
        break;
    case DataConnected:
        //E22
        //A0: do nothing
        break;
    case Connect:
        if (status)
        {
            //E19
            switch (_currentState)
            {
            case Closed:
            case OpenIdle:
            case OpenWait:
                //A0: do nothing
                break;
            case Connecting:
                _currentState = OpenIdle;
                A13(destination);
                break;
            }
        }
        else
        {
            //E20
            switch (_currentState)
            {
            case Closed:
            case OpenIdle:
            case OpenWait:
                //A0: do nothing
                break;
            case Connecting:
                A5(destination);
                break;
            }
        }
        break;
    case Disconnect:
        //E21
        //A0: do nothing
        break;
    case Ack:
        //E23
        //A0: do nothing
        break;
    case Nack:
        //E24
        //A0: do nothing
        break;
    default:
        break;
        /* DataGroup and DataBroadcast should not appear here. If they do ignore them. */
    }
}

void TransportLayer::dataGroupIndication(uint16_t destination, HopCountType hopType, Priority priority, uint16_t source, TPDU& tpdu)
{
    if (_groupAddressTable == nullptr)
        return;

    uint16_t tsap = _groupAddressTable->getTsap(destination);
    if (tsap == 0)
        return;
    
    _applicationLayer.dataGroupIndication(hopType, priority, tsap, tpdu.apdu());
}

void TransportLayer::dataGroupConfirm(AckType ack, uint16_t source, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu, bool status)
{
    _applicationLayer.dataGroupConfirm(ack, hopType, priority, destination, tpdu.apdu(), status);
}

void TransportLayer::dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, TPDU& tpdu)
{
    _applicationLayer.dataBroadcastIndication(hopType, priority, source, tpdu.apdu());
}

void TransportLayer::dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu, bool status)
{
    _applicationLayer.dataBroadcastConfirm(ack, hopType, priority, tpdu.apdu(), status);
}

void TransportLayer::dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, TPDU& tpdu)
{
    _applicationLayer.dataSystemBroadcastIndication(hopType, priority, source, tpdu.apdu());
}

void TransportLayer::dataSystemBroadcastConfirm(AckType ack, HopCountType hopType, TPDU& tpdu, Priority priority, bool status)
{
    _applicationLayer.dataSystemBroadcastConfirm(hopType, priority, tpdu.apdu(), status);
}

void TransportLayer::dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    if (_groupAddressTable == nullptr)
        return;

    uint16_t groupAdress = _groupAddressTable->getGroupAddress(tsap);
    TPDU& tpdu = apdu.frame().tpdu();
    _networkLayer->dataGroupRequest(ack, groupAdress, hopType, priority, tpdu);
}

void TransportLayer::dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu)
{
    TPDU& tpdu = apdu.frame().tpdu();
    _networkLayer->dataBroadcastRequest(ack, hopType, priority, tpdu);
}

void TransportLayer::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu)
{
    TPDU& tpdu = apdu.frame().tpdu();
    return _networkLayer->dataSystemBroadcastRequest(ack, hopType, priority, tpdu);
}

void TransportLayer::dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu)
{
    //print.print("-> TL  ");
    //apdu.printPDU();
    TPDU& tpdu = apdu.frame().tpdu();
    _networkLayer->dataIndividualRequest(ack, destination, hopType, priority, tpdu);
}

void TransportLayer::connectRequest(uint16_t destination, Priority priority)
{
    //E25
    switch (_currentState)
    {
    case Closed:
        _currentState = Connecting;
        A12(destination, priority);
        break;
    case OpenIdle:
    case OpenWait:
    case Connecting:
        _currentState = Closed;
        A6(destination);
        break;
    }
}

void TransportLayer::disconnectRequest(uint16_t tsap, Priority priority)
{
    //E26
    switch (_currentState)
    {
    case Closed:
        A15(priority, tsap);
        break;
    case OpenIdle:
    case OpenWait:
    case Connecting:
        _currentState = Closed;
        A14(tsap, priority);
        break;
    }
}

void TransportLayer::dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu)
{
    //print.print("-> TL  ");
    //apdu.printPDU();
    //E15
    switch (_currentState)
    {
    case Closed:
        //A0
        break;
    case OpenIdle:
        _currentState = OpenWait;
        A7(priority, apdu);
        break;
    case OpenWait:
    case Connecting:
        A11(tsap, priority, apdu);
        break;
    default:
        break;
    }
}

void TransportLayer::connectionTimeoutIndication()
{
    //E16
    switch (_currentState)
    {
    case Closed:
        //A0: do nothing
        break;
    case OpenIdle:
    case OpenWait:
    case Connecting:
        _currentState = Closed;
        A6(_connectionAddress);
        break;
    }
}

void TransportLayer::ackTimeoutIndication()
{
    if (_repCount < _maxRepCount)
    {
        //E17
        switch (_currentState)
        {
        case Closed:
        case OpenIdle:
        case Connecting:
            //A0: do nothing
            break;
        case OpenWait:
            A9();
            break;
        }
    }
    else
    {
        //E18
        switch (_currentState)
        {
        case Closed:
        case OpenIdle:
        case Connecting:
            //A0: do nothing
            break;
        case OpenWait:
            _currentState = Closed;
            A6(_connectionAddress);
            break;
        }
    }
}

// Note: we should probably also add the TSAP as argument if would support multiple concurrent connections
uint8_t TransportLayer::getTpciSeqNum()
{
    // Return seqNum that would be used for sending next frame
    // together with the TPDU type.
    return ((_seqNoSend & 0xF) << 2);
}

// Note: we should probably also add the TSAP as argument if would support multiple concurrent connections
uint16_t TransportLayer::getConnectionAddress()
{
    return _connectionAddress;
}

void TransportLayer::loop()
{
    uint32_t milliseconds = millis();
    if (_connectionTimeoutEnabled 
        && (milliseconds - _connectionTimeoutStartMillis) > _connectionTimeoutMillis)
        connectionTimeoutIndication();

    if (_ackTimeoutEnabled
        && (milliseconds - _ackTimeoutStartMillis) > _ackTimeoutMillis)
        ackTimeoutIndication();

    if (_savedConnectingValid)
    {
        //retry saved event
        _savedConnectingValid = false;
        dataConnectedRequest(_savedTsapConnecting, _savedPriorityConnecting, _savedFrameConnecting.apdu());
    }
}

void TransportLayer::sendControlTelegram(TpduType pduType, uint8_t seqNo)
{
    CemiFrame frame(0);
    TPDU& tpdu = frame.tpdu();
    tpdu.type(pduType);
    tpdu.sequenceNumber(seqNo);
    _networkLayer->dataIndividualRequest(AckRequested, _connectionAddress, NetworkLayerParameter,
        SystemPriority, tpdu);
}

void TransportLayer::A0()
{
    /* do nothing */
}

void TransportLayer::A1(uint16_t source)
{
    _connectionAddress = source;
    _applicationLayer.connectIndication(source);
    _seqNoSend = 0;
    _seqNoRecv = 0;
    enableConnectionTimeout();
}

void incSeqNr(uint8_t& seqNr)
{
    seqNr += 1;
    if (seqNr > 0xf)
        seqNr = 0;
}

void TransportLayer::A2(uint16_t source, Priority priority, APDU& apdu)
{
    sendControlTelegram(Ack, _seqNoRecv);
    incSeqNr(_seqNoRecv);
    _applicationLayer.dataConnectedIndication(priority, source, apdu);
    enableConnectionTimeout();
}

void TransportLayer::A3(uint16_t source, Priority priority, TPDU& recTpdu)
{
    sendControlTelegram(Ack, recTpdu.sequenceNumber());
    enableConnectionTimeout();
}

void TransportLayer::A4(uint16_t source, Priority priority, TPDU& recTpdu)
{
    sendControlTelegram(Nack, recTpdu.sequenceNumber());
    enableConnectionTimeout();
}

void TransportLayer::A5(uint16_t tsap)
{
    _applicationLayer.disconnectIndication(tsap);
    disableConnectionTimeout();
    disableAckTimeout();
}

void TransportLayer::A6(uint16_t tsap)
{
    sendControlTelegram(Disconnect, 0);
    _applicationLayer.disconnectIndication(tsap);
    disableConnectionTimeout();
    disableAckTimeout();
}

void TransportLayer::A7(Priority priority, APDU& apdu)
{
    _savedPriority = priority;
    TPDU& tpdu = apdu.frame().tpdu();
    tpdu.type(DataConnected);
    tpdu.sequenceNumber(_seqNoSend);
    _savedFrame = apdu.frame();
    _networkLayer->dataIndividualRequest(AckRequested, _connectionAddress, NetworkLayerParameter, priority, tpdu);
    _repCount = 0;
    enableAckTimeout();
    enableConnectionTimeout();
}

void TransportLayer::A8()
{
    disableAckTimeout();
    incSeqNr(_seqNoSend);
    _applicationLayer.dataConnectedConfirm(0);
    enableConnectionTimeout();
}

void TransportLayer::A9()
{
    TPDU& tpdu = _savedFrame.tpdu();
    // tpdu is still initialized from last send
    _networkLayer->dataIndividualRequest(AckRequested, _connectionAddress, NetworkLayerParameter, _savedPriority, tpdu);
    _repCount += 1;
    enableAckTimeout();
    enableConnectionTimeout();
}

void TransportLayer::A10(uint16_t source)
{
    CemiFrame frame(0);
    TPDU& tpdu = frame.tpdu();
    tpdu.type(Disconnect);
    tpdu.sequenceNumber(0);
    _networkLayer->dataIndividualRequest(AckRequested, source, NetworkLayerParameter, SystemPriority, tpdu);
}

void TransportLayer::A11(uint16_t tsap, Priority priority, APDU& apdu)
{
    _savedTsapConnecting = tsap;
    _savedPriorityConnecting = priority;
    _savedFrameConnecting = apdu.frame();
    _savedConnectingValid = true;
}

void TransportLayer::A12(uint16_t destination, Priority priority)
{
    _connectionAddress = destination;
    CemiFrame frame(0);
    TPDU& tpdu = frame.tpdu();
    tpdu.type(Connect);
    _networkLayer->dataIndividualRequest(AckRequested, destination, NetworkLayerParameter, priority, tpdu);
    _seqNoRecv = 0;
    _seqNoSend = 0;
    enableConnectionTimeout();
}

void TransportLayer::A13(uint16_t destination)
{
    _applicationLayer.connectConfirm(destination, 0, true);
}

void TransportLayer::A14(uint16_t tsap, Priority priority)
{
    CemiFrame frame(0);
    TPDU& tpdu = frame.tpdu();
    tpdu.type(Disconnect);
    tpdu.sequenceNumber(0);
    _networkLayer->dataIndividualRequest(AckRequested, _connectionAddress, NetworkLayerParameter, SystemPriority, tpdu);
    _applicationLayer.disconnectConfirm(priority, tsap, true);
    disableConnectionTimeout();
    disableAckTimeout();
}

void TransportLayer::A15(Priority priority, uint16_t tsap)
{
    _applicationLayer.disconnectConfirm(priority, tsap, true);
    disableConnectionTimeout();
    disableAckTimeout();
}

void TransportLayer::enableConnectionTimeout()
{
    _connectionTimeoutStartMillis = millis();
    _connectionTimeoutEnabled = true;
}

void TransportLayer::disableConnectionTimeout()
{
    _connectionTimeoutEnabled = false;
}

void TransportLayer::enableAckTimeout()
{
    _ackTimeoutStartMillis = millis();
    _ackTimeoutEnabled = true;
}

void TransportLayer::disableAckTimeout()
{
    _ackTimeoutEnabled = false;
}
