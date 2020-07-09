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
      _dlLayerPrimary(_deviceObj, _ipParameters, _netLayer, _platform),
      _dlLayerSecondary(_deviceObj, _netLayer, platform)
#ifdef USE_CEMI_SERVER
      ,
      _cemiServer(*this)
#endif
{
    _netLayer.dataLinkLayer(_dlLayerSecondary);
#ifdef USE_CEMI_SERVER
    _cemiServer.dataLinkLayer(_dlLayer);
    _dlLayer.cemiServer(_cemiServer);
    _memory.addSaveRestore(&_cemiServerObject);
#endif
    _memory.addSaveRestore(&_ipParameters);

    // Set Mask Version in Device Object depending on the BAU
    _deviceObj.maskVersion(0x091A);

    // Set which interface objects are available in the device object
    // This differs from BAU to BAU with different medium types.
    // See PID_IO_LIST
    Property* prop = _deviceObj.property(PID_IO_LIST);
    prop->write(1, (uint16_t) OT_DEVICE);
    prop->write(2, (uint16_t) OT_ADDR_TABLE);
    prop->write(3, (uint16_t) OT_ASSOC_TABLE);
    prop->write(4, (uint16_t) OT_GRP_OBJ_TABLE);
    prop->write(5, (uint16_t) OT_APPLICATION_PROG);
    prop->write(6, (uint16_t) OT_IP_PARAMETER);
#if defined(USE_DATASECURE) && defined(USE_CEMI_SERVER)
    prop->write(7, (uint16_t) OT_SECURITY);
    prop->write(8, (uint16_t) OT_CEMI_SERVER);
#elif defined(USE_DATASECURE)
    prop->write(7, (uint16_t) OT_SECURITY);
#elif defined(USE_CEMI_SERVER)
    prop->write(7, (uint16_t) OT_CEMI_SERVER);
#endif
}

InterfaceObject* Bau091A::getInterfaceObject(uint8_t idx)
{
    switch (idx)
    {
        case 0:
            return &_deviceObj;
        case 4:
            return &_appProgram;
        case 5: // would be app_program 2
            return nullptr;
        case 6:
            return &_ipParameters;
#if defined(USE_DATASECURE) && defined(USE_CEMI_SERVER)
        case 7:
            return &_secIfObj;
        case 8:
            return &_cemiServerObject;
#elif defined(USE_CEMI_SERVER)
        case 7:
            return &_cemiServerObject;
#elif defined(USE_DATASECURE)
        case 7:
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
    BauSystemB::doMasterReset(eraseCode, channel);

    _ipParameters.masterReset(eraseCode, channel);
}

DataLinkLayer& Bau091A::dataLinkLayer()
{
    return _dlLayerSecondary;
}

#endif
