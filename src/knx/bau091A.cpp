#include "config.h"
#if MASK_VERSION == 0x091A

#include "bau091A.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

using namespace std;

Bau091A::Bau091A(Platform& platform)
    : BauSystemBCoupler(platform),
      _routerObj(memory()),
      _ipParameters(_deviceObj, platform),
      _dlLayerPrimary(_deviceObj, _ipParameters, _netLayer.getPrimaryInterface(), _platform, (DataLinkLayerCallbacks*) this),
      _dlLayerSecondary(_deviceObj, _netLayer.getSecondaryInterface(), platform, (ITpUartCallBacks&) *this, (DataLinkLayerCallbacks*) this),
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
    _cemiServer.dataLinkLayer(_dlLayerSecondary); // Secondary I/F is the important one!
    _dlLayerSecondary.cemiServer(_cemiServer);
    _memory.addSaveRestore(&_cemiServerObject);
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

InterfaceObject* Bau091A::getInterfaceObject(ObjectType objectType, uint8_t objectInstance)
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
}

void Bau091A::loop()
{
    _dlLayerPrimary.loop();
    _dlLayerSecondary.loop();
    BauSystemBCoupler::loop();
}

bool Bau091A::isAckRequired(uint16_t address, bool isGrpAddr)
{
    if (isGrpAddr)
    {
        // ACK for broadcasts
        if (address == 0)
            return true;

        // is group address in filter table? ACK if yes.
        return _routerObj.isGroupAddressInFilterTable(address);
    }
    else
    {
        return _netLayer.isRoutedIndividualAddress(address);
    }

    return false;
}

#endif
