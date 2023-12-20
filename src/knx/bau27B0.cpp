#include "config.h"
#if MASK_VERSION == 0x27B0

#include "bau27B0.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

using namespace std;

Bau27B0::Bau27B0(Platform& platform)
    : BauSystemBDevice(platform),
      _dlLayer(_deviceObj, _rfMediumObj, _netLayer.getInterface(), _platform)
#ifdef USE_CEMI_SERVER
    , _cemiServer(*this)
#endif      
{
    _netLayer.getInterface().dataLinkLayer(_dlLayer);
    _memory.addSaveRestore(&_rfMediumObj);
#ifdef USE_CEMI_SERVER
    _cemiServerObject.setMediumTypeAsSupported(DptMedium::KNX_RF);
    _cemiServer.dataLinkLayer(_dlLayer);
    _dlLayer.cemiServer(_cemiServer);
    _memory.addSaveRestore(&_cemiServerObject);
#endif

    // Set Mask Version in Device Object depending on the BAU
    _deviceObj.maskVersion(0x27B0);

    // Set the maximum APDU length
    // ETS will consider this value while programming the device
    // For KNX-RF we use a smallest allowed value for now,
    // although long frame are also supported by the implementation.
    // Needs some experimentation.
    _deviceObj.maxApduLength(15);

    // Set which interface objects are available in the device object
    // This differs from BAU to BAU with different medium types.
    // See PID_IO_LIST
    Property* prop = _deviceObj.property(PID_IO_LIST);
    prop->write(1, (uint16_t) OT_DEVICE);
    prop->write(2, (uint16_t) OT_ADDR_TABLE);
    prop->write(3, (uint16_t) OT_ASSOC_TABLE);
    prop->write(4, (uint16_t) OT_GRP_OBJ_TABLE);
    prop->write(5, (uint16_t) OT_APPLICATION_PROG);
    prop->write(6, (uint16_t) OT_RF_MEDIUM);
#if defined(USE_DATASECURE) && defined(USE_CEMI_SERVER)
    prop->write(7, (uint16_t) OT_SECURITY);
    prop->write(8, (uint16_t) OT_CEMI_SERVER);
#elif defined(USE_DATASECURE)
    prop->write(7, (uint16_t) OT_SECURITY);
#elif defined(USE_CEMI_SERVER)
    prop->write(7, (uint16_t)OT_CEMI_SERVER);
#endif
}

// see KNX AN160 p.74 for mask 27B0
InterfaceObject* Bau27B0::getInterfaceObject(uint8_t idx)
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
            return &_rfMediumObj;
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

InterfaceObject* Bau27B0::getInterfaceObject(ObjectType objectType, uint8_t objectInstance)
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
        case OT_RF_MEDIUM:
            return &_rfMediumObj;
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

void Bau27B0::doMasterReset(EraseCode eraseCode, uint8_t channel)
{
    // Common SystemB objects
    BauSystemB::doMasterReset(eraseCode, channel);

    _rfMediumObj.masterReset(eraseCode, channel);
}

bool Bau27B0::enabled()
{
    return _dlLayer.enabled();
}

void Bau27B0::enabled(bool value)
{
    _dlLayer.enabled(value);
}

void Bau27B0::loop()
{
    _dlLayer.loop();
    BauSystemBDevice::loop();
#ifdef USE_CEMI_SERVER    
    _cemiServer.loop();
#endif    
}

void Bau27B0::domainAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* rfDoA,
                                                       const uint8_t* knxSerialNumber)
{
    // If the received serial number matches our serial number
    // then store the received RF domain address in the RF medium object
    if (!memcmp(knxSerialNumber, _deviceObj.propertyData(PID_SERIAL_NUMBER), 6))
        _rfMediumObj.rfDomainAddress(rfDoA);
}

void Bau27B0::domainAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* knxSerialNumber)
{
    // If the received serial number matches our serial number
    // then send a response with the current RF domain address stored in the RF medium object
    if (!memcmp(knxSerialNumber, _deviceObj.propertyData(PID_SERIAL_NUMBER), 6))
        _appLayer.domainAddressSerialNumberReadResponse(priority, hopType, secCtrl, _rfMediumObj.rfDomainAddress(), knxSerialNumber);
}

void Bau27B0::individualAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* knxSerialNumber)
{
    #pragma warning "individualAddressSerialNumberReadIndication is not available for rf"
}

void Bau27B0::domainAddressSerialNumberWriteLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* rfDoA,
                                                         const uint8_t* knxSerialNumber, bool status)
{
}

void Bau27B0::domainAddressSerialNumberReadLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* knxSerialNumber, bool status)
{
}


#endif // #ifdef USE_RF
