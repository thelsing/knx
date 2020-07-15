#include "config.h"
#include "bau091A.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

#ifdef USE_IP

using namespace std;

Bau091A::Bau091A(Platform& platform)
    : BauSystemBCoupler(platform),
      _ipParameters(_deviceObj, platform),
      _dlLayerPrimary(_deviceObj, _ipParameters, _netLayer.getEntity(0), _platform),
      _dlLayerSecondary(_deviceObj, _netLayer.getEntity(1), platform)
#ifdef USE_CEMI_SERVER
      ,
      _cemiServer(*this)
#endif
{
    _rtObjPrimary.property(PID_MEDIUM)->write((uint8_t) DptMedium::KNX_IP);
    _rtObjSecondary.property(PID_MEDIUM)->write((uint8_t) DptMedium::KNX_TP1);

    _rtObjPrimary.property(PID_OBJECT_INDEX)->write((uint8_t) 1);
    _rtObjSecondary.property(PID_OBJECT_INDEX)->write((uint8_t) 2);

    _netLayer.getEntity(0).dataLinkLayer(_dlLayerPrimary);
    _netLayer.getEntity(1).dataLinkLayer(_dlLayerSecondary);

#ifdef USE_CEMI_SERVER
    _cemiServer.dataLinkLayer(_dlLayerSecondary); // Secondary I/F is the important one!
    _dlLayer.cemiServer(_cemiServer);
    _memory.addSaveRestore(&_cemiServerObject);
#endif

    _memory.addSaveRestore(&_rtObjPrimary);
    _memory.addSaveRestore(&_rtObjSecondary);

    _memory.addSaveRestore(&_ipParameters);

    // Set Mask Version in Device Object depending on the BAU
    _deviceObj.maskVersion(0x091A);

    // Set which interface objects are available in the device object
    // This differs from BAU to BAU with different medium types.
    // See PID_IO_LIST
    Property* prop = _deviceObj.property(PID_IO_LIST);
    prop->write(1, (uint16_t) OT_DEVICE);
    prop->write(2, (uint16_t) OT_ROUTER);
    prop->write(3, (uint16_t) OT_ROUTER);
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
            return &_rtObjPrimary;
        case 2:
            return &_rtObjSecondary;
        case 3:
            return &_appProgram;
        case 4:
            return &_ipParameters;
#if defined(USE_DATASECURE) && defined(USE_CEMI_SERVER)
        case 5:
            return &_secIfObj;
        case 6:
            return &_cemiServerObject;
#elif defined(USE_CEMI_SERVER)
        case 5:
            return &_cemiServerObject;
#elif defined(USE_DATASECURE)
        case 5:
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
            return objectInstance == 0 ? &_rtObjPrimary : &_rtObjSecondary;
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

#endif
