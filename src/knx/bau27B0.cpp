#if MEDIUM_TYPE == 2

#include "bau27B0.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

using namespace std;

Bau27B0::Bau27B0(Platform& platform)
    : BauSystemB(platform),
      _dlLayer(_deviceObj, _rfMediumObj, _addrTable, _netLayer, _platform)
#ifdef USE_CEMI_SERVER
    , _cemiServer(*this)
#endif      
{
    _netLayer.dataLinkLayer(_dlLayer);
    _memory.addSaveRestore(&_rfMediumObj);
#ifdef USE_CEMI_SERVER
    _cemiServer.dataLinkLayer(_dlLayer);
    _dlLayer.cemiServer(_cemiServer);
    _memory.addSaveRestore(&_cemiServerObject);
#endif

    // Set Mask Version in Device Object depending on the BAU
    uint16_t maskVersion;
    popWord(maskVersion, _descriptor);
    _deviceObj.maskVersion(maskVersion);

    // Set the maximum APDU length
    // ETS will consider this value while programming the device
    // For KNX-RF we use a smallest allowed value for now,
    // although long frame are also supported by the implementation.
    // Needs some experimentation.
    _deviceObj.maxApduLength(15);

    // Set which interface objects are available in the device object
    // This differs from BAU to BAU with different medium types.
    // See PID_IO_LIST
    _deviceObj.ifObj(_ifObjs);
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
#ifdef USE_CEMI_SERVER
        case 7:
            return &_cemiServerObject;
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
#ifdef USE_CEMI_SERVER
        case OT_CEMI_SERVER:
            return &_cemiServerObject;
#endif            
        default:
            return nullptr;
    }
}

uint8_t* Bau27B0::descriptor()
{
    return _descriptor;
}

DataLinkLayer& Bau27B0::dataLinkLayer()
{
    return _dlLayer;
}

void Bau27B0::enabled(bool value)
{
    ::BauSystemB::enabled(value);
    //_tunnelInterface.enabled(value);
}

void Bau27B0::loop()
{
    ::BauSystemB::loop();
    //_tunnelInterface.loop();
}

void Bau27B0::domainAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, uint8_t* rfDoA,
                                                          uint8_t* knxSerialNumber)
{
    uint8_t curSerialNumber[6];
    pushWord(_deviceObj.manufacturerId(), &curSerialNumber[0]);
    pushInt(_deviceObj.bauNumber(), &curSerialNumber[2]);

    // If the received serial number matches our serial number
    // then store the received RF domain address in the RF medium object
    if (!memcmp(knxSerialNumber, curSerialNumber, 6))
        _rfMediumObj.rfDomainAddress(rfDoA);
}

void Bau27B0::domainAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, uint8_t* knxSerialNumber)
{
    uint8_t curSerialNumber[6];
    pushWord(_deviceObj.manufacturerId(), &curSerialNumber[0]);
    pushInt(_deviceObj.bauNumber(), &curSerialNumber[2]);

    // If the received serial number matches our serial number
    // then send a response with the current RF domain address stored in the RF medium object
    if (!memcmp(knxSerialNumber, curSerialNumber, 6))
        _appLayer.domainAddressSerialNumberReadResponse(priority, hopType, _rfMediumObj.rfDomainAddress(), knxSerialNumber);
}

void Bau27B0::individualAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, uint16_t newIndividualAddress,
                                                          uint8_t* knxSerialNumber)
{
    uint8_t curSerialNumber[6];
    pushWord(_deviceObj.manufacturerId(), &curSerialNumber[0]);
    pushInt(_deviceObj.bauNumber(), &curSerialNumber[2]);

    // If the received serial number matches our serial number
    // then store the received new individual address in the device object
    if (!memcmp(knxSerialNumber, curSerialNumber, 6))
        _deviceObj.induvidualAddress(newIndividualAddress);
}

void Bau27B0::individualAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, uint8_t* knxSerialNumber)
{
    uint8_t curSerialNumber[6];
    pushWord(_deviceObj.manufacturerId(), &curSerialNumber[0]);
    pushInt(_deviceObj.bauNumber(), &curSerialNumber[2]);

    // If the received serial number matches our serial number
    // then send a response with the current RF domain address stored in the RF medium object and the serial number
    if (!memcmp(knxSerialNumber, curSerialNumber, 6))
        _appLayer.IndividualAddressSerialNumberReadResponse(priority, hopType, _rfMediumObj.rfDomainAddress(), knxSerialNumber);
}

#endif // #if MEDIUM_TYPE == 2
