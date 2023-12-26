#include "config.h"
#if MASK_VERSION == 0x57B0

#include "bau57B0.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

using namespace std;

Bau57B0::Bau57B0(Platform& platform)
    : BauSystemBDevice(platform),
      _ipParameters(_deviceObj, platform),
      _dlLayer(_deviceObj, _ipParameters, _netLayer.getInterface(), _platform, (DataLinkLayerCallbacks*) this),
      DataLinkLayerCallbacks()
#ifdef USE_CEMI_SERVER
      ,
      _cemiServer(*this)
#endif
{
    _netLayer.getInterface().dataLinkLayer(_dlLayer);
#ifdef USE_CEMI_SERVER
    _cemiServerObject.setMediumTypeAsSupported(DptMedium::KNX_IP);
    _cemiServer.dataLinkLayer(_dlLayer);
    _dlLayer.cemiServer(_cemiServer);
    _memory.addSaveRestore(&_cemiServerObject);
#endif
    _memory.addSaveRestore(&_ipParameters);

    // Set Mask Version in Device Object depending on the BAU
    _deviceObj.maskVersion(0x57B0);

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

InterfaceObject* Bau57B0::getInterfaceObject(uint8_t idx)
{
    switch (idx)
    {
        case 0:
            return &_deviceObj;
        case 1:
            return &_addrTable;
        case 2:
            return &_assocTable;
        case 3:
            return &_groupObjTable;
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

InterfaceObject* Bau57B0::getInterfaceObject(ObjectType objectType, uint8_t objectInstance)
{
    // We do not use it right now. 
    // Required for coupler mode as there are multiple router objects for example
    (void) objectInstance;

    switch (objectType)
    {
        case OT_DEVICE:
            return &_deviceObj;
        case OT_ADDR_TABLE:
            return &_addrTable;
        case OT_ASSOC_TABLE:
            return &_assocTable;
        case OT_GRP_OBJ_TABLE:
            return &_groupObjTable;
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

void Bau57B0::doMasterReset(EraseCode eraseCode, uint8_t channel)
{
    // Common SystemB objects
    BauSystemB::doMasterReset(eraseCode, channel);

    _ipParameters.masterReset(eraseCode, channel);
}

bool Bau57B0::enabled()
{
    return _dlLayer.enabled();
}

void Bau57B0::enabled(bool value)
{
    _dlLayer.enabled(value);
}

void Bau57B0::loop()
{
    _dlLayer.loop();
    BauSystemBDevice::loop();
#ifdef USE_CEMI_SERVER
    _cemiServer.loop();
#endif
}

#endif
