#include "config.h"

#include <cstring>
#include "router_object.h"
#include "bits.h"
#include "memory.h"
#include "data_property.h"
#include "callback_property.h"
#include "function_property.h"

// Filter Table Realization Type 3
// The Filter Table Realisation Type 3 shall be organised as a memory mapped bit-field of
// 65536 bits and thus 8 192 octets. Each bit shall uniquely correspond to one Group Address.
// The full 16 bit KNX GA encoding range shall be supported.
//
// octet_address = GA_value div 8
// bit_position = GA_value mod 8
static constexpr uint16_t kFilterTableSize = 65536 / 8; //  Each group address is represented by one bit

enum RouteTableServices
{
    ClearRoutingTable = 0x01, // no info bytes
    SetRoutingTable = 0x02,   // no info bytes
    ClearGroupAddress = 0x03, // 4 bytes: start address and end address
    SetGroupAddress = 0x04,   // 4 bytes: start address and end address
};

RouterObject::RouterObject(Memory& memory)
    : TableObject(memory)
{
}

void RouterObject::initialize1x(DptMedium mediumType, uint16_t maxApduSize)
{
    // Object index property is not included for coupler model 1.x, so value is "don't care".
    initialize(CouplerModel::Model_1x, 200, mediumType, RouterObjectType::Single, maxApduSize);
}

void RouterObject::initialize20(uint8_t objIndex, DptMedium mediumType, RouterObjectType rtType, uint16_t maxApduSize)
{
    initialize(CouplerModel::Model_20, objIndex, mediumType, rtType, maxApduSize);
}

void RouterObject::initialize(CouplerModel model, uint8_t objIndex, DptMedium mediumType, RouterObjectType rtType, uint16_t maxApduSize)
{
    bool useHopCount = false;
    bool useTable = true;

    if (model == CouplerModel::Model_20)
    {
        useHopCount = (rtType == RouterObjectType::Primary);
        useTable = (rtType == RouterObjectType::Secondary);
    }

    // These properties are always present
    Property* fixedProperties[] =
    {
        new DataProperty( PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t) OT_ROUTER ),
        new DataProperty( PID_MEDIUM_STATUS, false, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0, (uint16_t) 0 ), // 0 means communication is possible, could be set by datalink layer or bau to 1 (comm impossible)
        new DataProperty( PID_MAX_APDU_LENGTH_ROUTER, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, maxApduSize ),
    };
    uint8_t fixedPropertiesCount = sizeof(fixedProperties) / sizeof(Property*);

    // Only present if coupler model is 1.x
    Property* model1xProperties[] =
    {
        // TODO: implement filtering based on this config here
        new DataProperty( PID_MAIN_LCCONFIG, true, PDT_BITSET8, 1, ReadLv3 | WriteLv0, (uint8_t) 0 ), // Primary: data individual (connless and connorient) + broadcast
        new DataProperty( PID_SUB_LCCONFIG, true, PDT_BITSET8, 1, ReadLv3 | WriteLv0, (uint8_t) 0 ), // Secondary: data individual (connless and connorient) + broadcast
        new DataProperty( PID_MAIN_LCGRPCONFIG, true, PDT_BITSET8, 1, ReadLv3 | WriteLv0, (uint8_t) 0 ), // Primary: data group
        new DataProperty( PID_SUB_LCGRPCONFIG, true, PDT_BITSET8, 1, ReadLv3 | WriteLv0, (uint8_t) 0 ), // Secondary: data group
    };
    uint8_t model1xPropertiesCount = sizeof(model1xProperties) / sizeof(Property*);

    // Only present if coupler model is 2.0
    // One router object per interface, currently only TP1/RF coupler specified
    Property* model20Properties[] =
    {
        new DataProperty( PID_OBJECT_INDEX, false, PDT_UNSIGNED_CHAR, 1, ReadLv3 | WriteLv0, objIndex ), // Must be set by concrete BAUxxxx!
        new DataProperty( PID_MEDIUM, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0, (uint8_t) mediumType ),
    };
    uint8_t model20PropertiesCount = sizeof(model20Properties) / sizeof(Property*);

    Property* tableProperties[] =
    {
        new DataProperty( PID_COUPLER_SERVICES_CONTROL, true, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0, (uint8_t) 0), // written by ETS TODO: implement
        new DataProperty( PID_FILTER_TABLE_USE, true, PDT_BINARY_INFORMATION, 1, ReadLv3 | WriteLv0, (uint16_t) 0 ), // default: invalid filter table, do not use, written by ETS
        new FunctionProperty<RouterObject>(this, PID_ROUTETABLE_CONTROL,
                // Command Callback of PID_ROUTETABLE_CONTROL
                [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                    obj->functionRouteTableControl(true, data, length, resultData, resultLength);
                },
                // State Callback of PID_ROUTETABLE_CONTROL
                [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                    obj->functionRouteTableControl(false, data, length, resultData, resultLength);
                })
    };
    uint8_t tablePropertiesCount = sizeof(tableProperties) / sizeof(Property*);

    size_t allPropertiesCount = fixedPropertiesCount;
    allPropertiesCount += (model == CouplerModel::Model_1x) ? model1xPropertiesCount : model20PropertiesCount;
    allPropertiesCount += useHopCount ? 1 : 0;
    allPropertiesCount += useTable ? tablePropertiesCount : 0;
    allPropertiesCount += ((mediumType == DptMedium::KNX_RF) || (mediumType == DptMedium::KNX_IP)) ? 1 : 0; // PID_RF_ENABLE_SBC and PID_IP_ENABLE_SBC

    Property* allProperties[allPropertiesCount];

    memcpy(&allProperties[0], &fixedProperties[0], sizeof(fixedProperties));

    uint8_t i = fixedPropertiesCount;

    if (model == CouplerModel::Model_1x)
    {
        memcpy(&allProperties[i], model1xProperties, sizeof(model1xProperties));
        i += model1xPropertiesCount;
    }
    else
    {
        memcpy(&allProperties[i], model20Properties, sizeof(model20Properties));
        i += model20PropertiesCount;
    }

    if (useHopCount)
    {
        // TODO: Primary side: 5 for line coupler, 4 for backbone coupler, only exists if secondary is open medium without hop count
        // Do we need to set a default value here or is it written by ETS?
        allProperties[i++] = new DataProperty( PID_HOP_COUNT, true, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t) 5);
    }

    if (useTable)
    {
        memcpy(&allProperties[i], tableProperties, sizeof(tableProperties));
        i += tablePropertiesCount;
    }

    if (mediumType == DptMedium::KNX_RF)
    {
        allProperties[i++] = new FunctionProperty<RouterObject>(this, PID_RF_ENABLE_SBC,
                                    // Command Callback of PID_RF_ENABLE_SBC
                                    [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                                       obj->functionRfEnableSbc(true, data, length, resultData, resultLength);
                                    },
                                    // State Callback of PID_RF_ENABLE_SBC
                                    [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                                       obj->functionRfEnableSbc(false, data, length, resultData, resultLength);
                                    });
    }
    else if (mediumType == DptMedium::KNX_IP)
    {
        allProperties[i++] = new FunctionProperty<RouterObject>(this, PID_IP_ENABLE_SBC,
                                    // Command Callback of PID_IP_ENABLE_SBC
                                    [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                                       obj->functionIpEnableSbc(true, data, length, resultData, resultLength);
                                    },
                                    // State Callback of PID_IP_ENABLE_SBC
                                    [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                                       obj->functionIpEnableSbc(false, data, length, resultData, resultLength);
                                    });
    }

    if (useTable)
        TableObject::initializeProperties(sizeof(allProperties), allProperties);
    else
        InterfaceObject::initializeProperties(sizeof(allProperties), allProperties);
}

const uint8_t* RouterObject::restore(const uint8_t* buffer)
{
    buffer = TableObject::restore(buffer);

    _filterTableGroupAddresses = (uint16_t*)data();

    return buffer;
}

void RouterObject::commandClearSetRoutingTable(bool bitIsSet)
{
    for (uint16_t i = 0; i < kFilterTableSize; i++)
    {
        data()[i] = bitIsSet ? 0xFF : 0x00;
    }
}

bool RouterObject::statusClearSetRoutingTable(bool bitIsSet)
{
    for (uint16_t i = 0; i < kFilterTableSize; i++)
    {
        if (data()[i] != (bitIsSet ? 0xFF : 0x00))
            return false;
    }
    return true;
}

void RouterObject::commandClearSetGroupAddress(uint16_t startAddress, uint16_t endAddress, bool bitIsSet)
{
    uint16_t startOctet = startAddress / 8;
    uint8_t startBitPosition = startAddress % 8;
    uint16_t endOctet = endAddress / 8;
    uint8_t endBitPosition = endAddress % 8;

    if (startOctet == endOctet)
    {
        for (uint8_t bitPos = startBitPosition; bitPos <= endBitPosition; bitPos++)
        {
            if (bitIsSet)
                data()[startOctet] |= 1 << bitPos;
            else
                data()[startOctet] &= ~(1 << bitPos);
        }
        return;
    }

    for (uint16_t i = startOctet; i <= endOctet; i++)
    {
        if (i == startOctet)
        {
            for (uint8_t bitPos = startBitPosition; bitPos <= 7; bitPos++)
            {
                if (bitIsSet)
                    data()[i] |= 1 << bitPos;
                else
                    data()[i] &= ~(1 << bitPos);
            }
        }
        else if (i == endOctet)
        {
            for (uint8_t bitPos = 0; bitPos <= endBitPosition; bitPos++)
            {
                if (bitIsSet)
                    data()[i] |= 1 << bitPos;
                else
                    data()[i] &= ~(1 << bitPos);
            }
        }
        else
        {
            if (bitIsSet)
                data()[i] = 0xFF;
            else
                data()[i] = 0x00;
        }
    }
}

bool RouterObject::statusClearSetGroupAddress(uint16_t startAddress, uint16_t endAddress, bool bitIsSet)
{
    uint16_t startOctet = startAddress / 8;
    uint8_t startBitPosition = startAddress % 8;
    uint16_t endOctet = endAddress / 8;
    uint8_t endBitPosition = endAddress % 8;

    if (startOctet == endOctet)
    {
        for (uint8_t bitPos = startBitPosition; bitPos <= endBitPosition; bitPos++)
        {
            if (bitIsSet)
            {
                if ((data()[startOctet] & (1 << bitPos)) == 0)
                    return false;
            }
            else
            {
                if ((data()[startOctet] & (1 << bitPos)) != 0)
                    return false;
            }
        }
        return true;
    }

    for (uint16_t i = startOctet; i <= endOctet; i++)
    {
        if (i == startOctet)
        {
            for (uint8_t bitPos = startBitPosition; bitPos <= 7; bitPos++)
            {
                if (bitIsSet)
                {
                    if ((data()[i] & (1 << bitPos)) == 0)
                        return false;
                }
                else
                {
                    if ((data()[i] & (1 << bitPos)) != 0)
                        return false;
                }
            }
        }
        else if (i == endOctet)
        {
            for (uint8_t bitPos = 0; bitPos <= endBitPosition; bitPos++)
            {
                if (bitIsSet)
                {
                    if ((data()[i] & (1 << bitPos)) == 0)
                        return false;
                }
                else
                {
                    if ((data()[i] & (1 << bitPos)) != 0)
                        return false;
                }
            }
        }
        else
        {
            if (data()[i] != (bitIsSet ? 0xFF : 0x00))
                return false;
        }
    }

    return true;
}

void RouterObject::functionRouteTableControl(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
{
    RouteTableServices srvId = (RouteTableServices) data[1];

    if (isCommand)
    {
        switch(srvId)
        {
            case ClearRoutingTable:
                commandClearSetRoutingTable(false);
                resultData[0] = ReturnCodes::Success;
                resultData[1] = srvId;
                resultLength = 2;
                return;
            case SetRoutingTable:
                commandClearSetRoutingTable(true);
                resultData[0] = ReturnCodes::Success;
                resultData[1] = srvId;
                resultLength = 2;
                return;
            case ClearGroupAddress:
            {
                uint16_t startAddress;
                uint16_t endAddress;
                popWord(startAddress, &data[2]);
                popWord(endAddress, &data[4]);
                commandClearSetGroupAddress(startAddress, endAddress, false);
                resultData[0] = ReturnCodes::Success;
                resultData[1] = srvId;
                pushWord(startAddress, &resultData[2]);
                pushWord(endAddress, &resultData[4]);
                resultLength = 6;
                return;
            }
            case SetGroupAddress:
            {
                uint16_t startAddress;
                uint16_t endAddress;
                popWord(startAddress, &data[2]);
                popWord(endAddress, &data[4]);
                commandClearSetGroupAddress(startAddress, endAddress, true);
                resultData[0] = ReturnCodes::Success;
                resultData[1] = srvId;
                pushWord(startAddress, &resultData[2]);
                pushWord(endAddress, &resultData[4]);
                resultLength = 6;
                return;
            }
        }
    }
    else
    {
        switch(srvId)
        {
            case ClearRoutingTable:
                resultData[0] = statusClearSetRoutingTable(false) ? ReturnCodes::Success : ReturnCodes::GenericError;
                resultData[1] = srvId;
                resultLength = 2;
                return;
            case SetRoutingTable:
                resultData[0] = statusClearSetRoutingTable(true) ? ReturnCodes::Success : ReturnCodes::GenericError;
                resultData[1] = srvId;
                resultLength = 2;
                return;
            case ClearGroupAddress:
            {
                uint16_t startAddress;
                uint16_t endAddress;
                popWord(startAddress, &data[2]);
                popWord(endAddress, &data[4]);
                resultData[0] = statusClearSetGroupAddress(startAddress, endAddress, false) ? ReturnCodes::Success : ReturnCodes::GenericError;
                resultData[1] = srvId;
                pushWord(startAddress, &resultData[2]);
                pushWord(endAddress, &resultData[4]);
                resultLength = 6;
                return;
            }
            case SetGroupAddress:
            {
                uint16_t startAddress;
                uint16_t endAddress;
                popWord(startAddress, &data[2]);
                popWord(endAddress, &data[4]);
                resultData[0] = statusClearSetGroupAddress(startAddress, endAddress, true) ? ReturnCodes::Success : ReturnCodes::GenericError;
                resultData[1] = srvId;
                pushWord(startAddress, &resultData[2]);
                pushWord(endAddress, &resultData[4]);
                resultLength = 6;
                return;
            }
        }
    }

    // We should not get here
    resultData[0] = ReturnCodes::GenericError;
    resultData[1] = srvId;
    resultLength = 2;
}

void RouterObject::functionRfEnableSbc(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
{
    if (isCommand)
    {
        _rfSbcRoutingEnabled = (data[0] == 1) ? true : false;
    }

    resultData[0] = ReturnCodes::Success;
    resultData[1] = _rfSbcRoutingEnabled ? 1 : 0;
    resultLength = 2;
}

bool RouterObject::isRfSbcRoutingEnabled()
{
    return _rfSbcRoutingEnabled;
}

// TODO: check if IP SBC works the same way, just copied from RF
void RouterObject::functionIpEnableSbc(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
{
    if (isCommand)
    {
        _ipSbcRoutingEnabled = (data[0] == 1) ? true : false;
    }

    resultData[0] = ReturnCodes::Success;
    resultData[1] = _ipSbcRoutingEnabled ? 1 : 0;
    resultLength = 2;
}

// TODO: check if IP SBC works the same way, just copied from RF
bool RouterObject::isIpSbcRoutingEnabled()
{
    return _ipSbcRoutingEnabled;
}

void RouterObject::beforeStateChange(LoadState& newState)
{
    if (newState != LS_LOADED)
        return;

    _filterTableGroupAddresses = (uint16_t*)data();
}

void RouterObject::masterReset(EraseCode eraseCode, uint8_t channel)
{
    if (eraseCode == FactoryReset)
    {
        // TODO: handle different erase codes
        println("Factory reset of router object with filter table requested.");
    }
}

bool RouterObject::isGroupAddressInFilterTable(uint16_t groupAddress)
{
    if (loadState() != LS_LOADED)
        return false;

    uint8_t filterTableUse = 0x00;
    if (property(PID_FILTER_TABLE_USE)->read(filterTableUse) == 0)
        return false;

    if ((filterTableUse&0x01) == 1)
    {
        // octet_address = GA_value div 8
        // bit_position = GA_value mod 8
        uint16_t octetAddress = groupAddress / 8;
        uint8_t bitPosition = groupAddress % 8;

        return (data()[octetAddress] & (1 << bitPosition)) == (1 << bitPosition);
    }

    return false;
}
