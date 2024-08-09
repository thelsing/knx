#include "config.h"
#if MASK_VERSION == 0x091A

#include "bau091A.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

using namespace std;

/* ToDos
Announce the line status of sec side 03_05_01 4.4.3
implement PID_COUPLER_SERVICES_CONTROL 03_05_01 4.4.7
*/

Bau091A::Bau091A(Platform& platform)
    : BauSystemBCoupler(platform),
      _routerObj(memory(), 0x200, 0x2000),  // the Filtertable of 0x091A IP Routers is fixed at 0x200 and 0x2000 long
      _ipParameters(_deviceObj, platform),
      _dlLayerPrimary(_deviceObj, _ipParameters, _netLayer.getPrimaryInterface(), _platform, *this, (DataLinkLayerCallbacks*) this),
      _dlLayerSecondary(_deviceObj, _netLayer.getSecondaryInterface(), platform, *this, (ITpUartCallBacks&) *this, (DataLinkLayerCallbacks*) this),
      DataLinkLayerCallbacks()
#ifdef USE_CEMI_SERVER
      ,
      _cemiServer(*this)
#endif
{
    // Before accessing anything of the router object they have to be initialized according to the used medium
    // Coupler model 1.x
    _routerObj.initialize1x(DptMedium::KNX_IP, 220);

    // Mask 091A uses older coupler model 1.x which only uses one router object
    _netLayer.rtObj(_routerObj);

    _netLayer.getPrimaryInterface().dataLinkLayer(_dlLayerPrimary);
    _netLayer.getSecondaryInterface().dataLinkLayer(_dlLayerSecondary);

#ifdef USE_CEMI_SERVER
    _cemiServerObject.setMediumTypeAsSupported(DptMedium::KNX_IP);
    _cemiServerObject.setMediumTypeAsSupported(DptMedium::KNX_TP1);
    _cemiServer.dataLinkLayerPrimary(_dlLayerPrimary);
    _cemiServer.dataLinkLayer(_dlLayerSecondary); // Secondary I/F is the important one!
    _dlLayerPrimary.cemiServer(_cemiServer);
    _dlLayerSecondary.cemiServer(_cemiServer);
    _memory.addSaveRestore(&_cemiServerObject);
    uint8_t count = 1;
    uint16_t suppCommModes = 0x0100;
    _cemiServerObject.writeProperty(PID_COMM_MODES_SUPPORTED, 1, (uint8_t*)&suppCommModes, count); // set the properties Bit 0 to 1 meaning "LinkLayer supported"
#endif

    _memory.addSaveRestore(&_routerObj);

    _memory.addSaveRestore(&_ipParameters);

    // Set Mask Version in Device Object depending on the BAU
    _deviceObj.maskVersion(0x091A);

    // Set which interface objects are available in the device object
    // This differs from BAU to BAU with different medium types.
    // See PID_IO_LIST
    Property* prop = _deviceObj.property(PID_IO_LIST);
    prop->write(1, (uint16_t) OT_DEVICE);
    prop->write(2, (uint16_t) OT_ROUTER);
    prop->write(3, (uint16_t) OT_APPLICATION_PROG);
    prop->write(4, (uint16_t) OT_IP_PARAMETER);
#if defined(USE_DATASECURE) && defined(USE_CEMI_SERVER)
    prop->write(5, (uint16_t) OT_SECURITY);
    prop->write(6, (uint16_t) OT_CEMI_SERVER);
#elif defined(USE_DATASECURE)
    prop->write(5, (uint16_t) OT_SECURITY);
#elif defined(USE_CEMI_SERVER)
    prop->write(5, (uint16_t) OT_CEMI_SERVER);
#endif
}

InterfaceObject* Bau091A::getInterfaceObject(uint8_t idx)
{
    switch (idx)
    {
        case 0:
            return &_deviceObj;
        case 1:
            return &_routerObj;
        case 2:
            return &_appProgram;
        case 3:
            return &_ipParameters;
#if defined(USE_DATASECURE) && defined(USE_CEMI_SERVER)
        case 4:
            return &_secIfObj;
        case 5:
            return &_cemiServerObject;
#elif defined(USE_CEMI_SERVER)
        case 4:
            return &_cemiServerObject;
#elif defined(USE_DATASECURE)
        case 4:
            return &_secIfObj;
#endif
        default:
            return nullptr;
    }
}

InterfaceObject* Bau091A::getInterfaceObject(ObjectType objectType, uint16_t objectInstance)
{
    // We do not use it right now. 
    // Required for coupler mode as there are multiple router objects for example
    (void) objectInstance;

    switch (objectType)
    {
        case OT_DEVICE:
            return &_deviceObj;
        case OT_ROUTER:
            return &_routerObj;
        case OT_APPLICATION_PROG:
            return &_appProgram;
        case OT_IP_PARAMETER:
            return &_ipParameters;
#ifdef USE_DATASECURE
        case OT_SECURITY:
            return &_secIfObj;
#endif
#ifdef USE_CEMI_SERVER
        case OT_CEMI_SERVER:
            return &_cemiServerObject;
#endif
        default:
            return nullptr;
    }
}

void Bau091A::doMasterReset(EraseCode eraseCode, uint8_t channel)
{
    // Common SystemB objects
    BauSystemBCoupler::doMasterReset(eraseCode, channel);

    _ipParameters.masterReset(eraseCode, channel);
    _routerObj.masterReset(eraseCode, channel);
}

bool Bau091A::enabled()
{
    return _dlLayerPrimary.enabled() && _dlLayerSecondary.enabled();
}

void Bau091A::enabled(bool value)
{
    _dlLayerPrimary.enabled(value);
    _dlLayerSecondary.enabled(value);

    // ToDo change frame repitition in the TP layer - but default is ok.
    //_dlLayerSecondary.setFrameRepetition(3,3);
}

void Bau091A::loop()
{
    _dlLayerPrimary.loop();
    _dlLayerSecondary.loop();
    BauSystemBCoupler::loop();
}

TPAckType Bau091A::isAckRequired(uint16_t address, bool isGrpAddr)
{
    //only called from TpUartDataLinkLayer
    TPAckType ack = TPAckType::AckReqNone;

    uint8_t lcconfig = LCCONFIG::PHYS_FRAME_ROUT | LCCONFIG::PHYS_REPEAT | LCCONFIG::BROADCAST_REPEAT | LCCONFIG::GROUP_IACK_ROUT | LCCONFIG::PHYS_IACK_NORMAL; // default value from spec. in case prop is not availible.
    Property* prop_lcconfig = _routerObj.property(PID_SUB_LCCONFIG);
    if(lcconfig)
        prop_lcconfig->read(lcconfig);

    if (isGrpAddr)
    {
        // ACK for broadcasts
        if (address == 0)
            ack = TPAckType::AckReqAck;

        if(lcconfig & LCCONFIG::GROUP_IACK_ROUT)
            // is group address in filter table? ACK if yes, No if not
            if(_netLayer.isRoutedGroupAddress(address, 1))
                ack = TPAckType::AckReqAck;
            else
                ack = TPAckType::AckReqNone;
        else
            // all are ACKED
            ack = TPAckType::AckReqAck;
#ifdef KNX_TUNNELING
        if(_dlLayerPrimary.isSentToTunnel(address, isGrpAddr))
            ack = TPAckType::AckReqAck;
#endif
    }
    else
    {
        if((lcconfig & LCCONFIG::PHYS_IACK) == LCCONFIG::PHYS_IACK_ALL)
            ack = TPAckType::AckReqAck;
        else if((lcconfig & LCCONFIG::PHYS_IACK) == LCCONFIG::PHYS_IACK_NACK)
            ack = TPAckType::AckReqNack;
        else
            if(_netLayer.isRoutedIndividualAddress(address, 1) || address == _deviceObj.individualAddress()) // Also ACK for our own individual address
                ack = TPAckType::AckReqAck;
            else
                ack = TPAckType::AckReqNone;

#ifdef KNX_TUNNELING
        if(_dlLayerPrimary.isSentToTunnel(address, isGrpAddr))
            ack = TPAckType::AckReqAck;
#endif

    }

    return ack;
}

bool Bau091A::configured()
{
    // _configured is set to true initially, if the device was configured with ETS it will be set to true after restart
    
    if (!_configured)
        return false;
    
    _configured = _routerObj.loadState() == LS_LOADED;
#ifdef USE_DATASECURE
    _configured &= _secIfObj.loadState() == LS_LOADED;
#endif
    
    return _configured;
}

IpDataLinkLayer* Bau091A::getPrimaryDataLinkLayer() {
    return (IpDataLinkLayer*)&_dlLayerPrimary;
}

TpUartDataLinkLayer* Bau091A::getSecondaryDataLinkLayer() {
    return (TpUartDataLinkLayer*)&_dlLayerSecondary;
}
#endif
