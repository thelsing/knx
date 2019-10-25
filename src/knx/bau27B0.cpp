#include "bau27B0.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

using namespace std;

Bau27B0::Bau27B0(Platform& platform)
    : BauSystemB(platform),
      _dlLayer(_deviceObj, _rfMediumObj, _addrTable, _netLayer, _platform)
{
    _netLayer.dataLinkLayer(_dlLayer);
    _memory.addSaveRestore(&_rfMediumObj);

    // Set Mask Version in Device Object depending on the BAU
    uint16_t maskVersion;
    popWord(maskVersion, _descriptor);
    _deviceObj.maskVersion(maskVersion);
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
