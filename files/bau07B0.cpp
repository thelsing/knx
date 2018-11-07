#include "bau07B0.h"
#include <string.h>
#include <stdio.h>


using namespace std;

Bau07B0::Bau07B0(Platform& platform): BauSystemB(platform),
    _dlLayer(_deviceObj, _addrTable, _netLayer, _platform)
{
    _netLayer.dataLinkLayer(_dlLayer);

    uint8_t descriptor[] = { 0x07, 0xb0 };
    _descriptor[0] = descriptor[0];
    _descriptor[1] = descriptor[1];
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
