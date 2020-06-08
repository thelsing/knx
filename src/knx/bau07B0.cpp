#include "bau07B0.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

#ifdef USE_TP

using namespace std;

Bau07B0::Bau07B0(Platform& platform)
    : BauSystemB(platform),
      _dlLayer(_deviceObj, _addrTable, _netLayer, _platform)
#ifdef USE_CEMI_SERVER
    , _cemiServer(*this)
#endif           
{
    _netLayer.dataLinkLayer(_dlLayer);
#ifdef USE_CEMI_SERVER
    _cemiServer.dataLinkLayer(_dlLayer);
    _dlLayer.cemiServer(_cemiServer);
    _memory.addSaveRestore(&_cemiServerObject);
#endif
#ifdef USE_DATASECURE
    _memory.addSaveRestore(&_secIfObj);
#endif

    // Set Mask Version in Device Object depending on the BAU
    _deviceObj.maskVersion(0x07B0);

    // Set which interface objects are available in the device object
    // This differs from BAU to BAU with different medium types.
    // See PID_IO_LIST
    Property* prop = _deviceObj.property(PID_IO_LIST);
    prop->write(1, OT_DEVICE);
    prop->write(2, OT_ADDR_TABLE);
    prop->write(3, OT_ASSOC_TABLE);
    prop->write(4, OT_GRP_OBJ_TABLE);
    prop->write(5, OT_APPLICATION_PROG);
#if defined(USE_DATASECURE) && defined(USE_CEMI_SERVER)
    prop->write(6, OT_SECURITY);
    prop->write(7, OT_CEMI_SERVER);
#elif defined(USE_DATASECURE)
    prop->write(6, OT_SECURITY);
#elif defined(USE_CEMI_SERVER)
    prop->write(6, OT_CEMI_SERVER);
#endif
}

InterfaceObject* Bau07B0::getInterfaceObject(uint8_t idx)
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
#if defined(USE_DATASECURE) && defined(USE_CEMI_SERVER)
        case 6:
            return &_secIfObj;
        case 7:
            return &_cemiServerObject;
#elif defined(USE_CEMI_SERVER)
        case 6:
            return &_cemiServerObject;
#elif defined(USE_DATASECURE)
        case 6:
            return &_secIfObj;
#endif
        default:
            return nullptr;
    }
}

InterfaceObject* Bau07B0::getInterfaceObject(ObjectType objectType, uint8_t objectInstance)
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

DataLinkLayer& Bau07B0::dataLinkLayer()
{
    return _dlLayer;
}

void Bau07B0::loop()
{
    ::BauSystemB::loop();
#ifdef USE_CEMI_SERVER    
    _cemiServer.loop();
#endif    
}

#endif
