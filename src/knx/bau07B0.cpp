#include "bau07B0.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

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

    // Set Mask Version in Device Object depending on the BAU
    uint16_t maskVersion;
    popWord(maskVersion, _descriptor);
    _deviceObj.maskVersion(maskVersion);

    // Set which interface objects are available in the device object
    // This differs from BAU to BAU with different medium types.
    // See PID_IO_LIST
    _deviceObj.ifObj(_ifObjs);
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
#ifdef USE_CEMI_SERVER
        case 6:
            return &_cemiServerObject;
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
#ifdef USE_CEMI_SERVER
        case OT_CEMI_SERVER:
            return &_cemiServerObject;
#endif            
        default:
            return nullptr;
    }
}

uint8_t* Bau07B0::descriptor()
{
    return _descriptor;
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