#include "config.h"

#include <cstring>
#include "router_object.h"
#include "bits.h"
#include "memory.h"
#include "data_property.h"
#include "callback_property.h"
#include "function_property.h"

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
    Property* properties[] =
    {
        new DataProperty( PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t) OT_ROUTER ),
        new DataProperty( PID_OBJECT_INDEX, false, PDT_UNSIGNED_CHAR, 1, ReadLv3 | WriteLv0 ), // Must be set by concrete BAUxxxx
        new DataProperty( PID_MEDIUM_STATUS, false, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0, (uint16_t) 0 ), // For now: communication on medium is always possible
        new DataProperty( PID_MAX_APDU_LENGTH_ROUTER, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t) 254 ), // For now: fixed size
        new DataProperty( PID_HOP_COUNT, true, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t) 5), // TODO: Primary side: 5 for line coupler, 4 for backbone coupler, only exists if secondary is open medium without hop count
        new DataProperty( PID_MEDIUM, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0 ), // Must be set by concrete BAUxxxx
        new DataProperty( PID_MCB_TABLE, false, PDT_GENERIC_08, 1, ReadLv3 | WriteLv0),
        new DataProperty( PID_FILTER_TABLE_USE, true, PDT_BINARY_INFORMATION, 1, ReadLv3 | WriteLv0, (uint16_t) 0 ), // default: invalid filter table, do not use
        new FunctionProperty<RouterObject>(this, PID_ROUTETABLE_CONTROL,
            // Command Callback of PID_ROUTETABLE_CONTROL
            [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                obj->functionRouteTableControl(true, data, length, resultData, resultLength);
            },
            // State Callback of PID_ROUTETABLE_CONTROL
            [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                obj->functionRouteTableControl(false, data, length, resultData, resultLength);
            }),
        new FunctionProperty<RouterObject>(this, PID_RF_ENABLE_SBC,
            // Command Callback of PID_RF_ENABLE_SBC
            [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                obj->functionRfEnableSbc(true, data, length, resultData, resultLength);
            },
            // State Callback of PID_RF_ENABLE_SBC
            [](RouterObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                obj->functionRfEnableSbc(false, data, length, resultData, resultLength);
            }),
    };

    TableObject::initializeProperties(sizeof(properties), properties);
}
const uint8_t* RouterObject::restore(const uint8_t* buffer)
{
    buffer = TableObject::restore(buffer);

    _filterTableGroupAddresses = (uint16_t*)data();

    return buffer;
}

void RouterObject::functionRouteTableControl(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
{
    bool isError = false;
    RouteTableServices srvId = (RouteTableServices) data[1];

    // Filter Table Realization Type 3
    // The Filter Table Realisation Type 3 shall be organised as a memory mapped bit-field of
    // 65536 bits and thus 8 192 octets. Each bit shall uniquely correspond to one Group Address.
    // The full 16 bit KNX GA encoding range shall be supported.
    //
    // octet_address = GA_value div 8
    // bit_position = GA_value mod 8

    if (isCommand)
    {
        switch(srvId)
        {
            case ClearRoutingTable:
            case SetRoutingTable:
            case ClearGroupAddress:
            case SetGroupAddress: break;
            default: isError = true;
        }
    }
    else
    {
        switch(srvId)
        {
            case ClearRoutingTable:
            case SetRoutingTable:
            case ClearGroupAddress:
            case SetGroupAddress: break;
            default: isError = true;
        }
    }

    if (isError)
    {
        resultData[0] = ReturnCodes::GenericError;
        resultData[1] = srvId;
        resultLength = 2;
    }
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

void RouterObject::beforeStateChange(LoadState& newState)
{
    if (newState != LS_LOADED)
        return;

    // calculate crc16-ccitt for PID_MCB_TABLE
    updateMcb();

    _filterTableGroupAddresses = (uint16_t*)data();
}

void RouterObject::updateMcb()
{
    uint8_t mcb[propertySize(PID_MCB_TABLE)];

    static constexpr uint32_t segmentSize = 8192;
    uint16_t crc16 = crc16Ccitt(data(), segmentSize);

    pushInt(segmentSize, &mcb[0]); // Segment size
    pushByte(0x00, &mcb[4]);       // CRC control byte -> 0: always valid -> according to coupler spec. it shall always be a valid CRC
    pushByte(0xFF, &mcb[5]);       // Read access 4 bits + Write access 4 bits (unknown: value taken from real coupler device)
    pushWord(crc16, &mcb[6]);      // CRC-16 CCITT of filter table

    property(PID_MCB_TABLE)->write(mcb);
}

void RouterObject::masterReset(EraseCode eraseCode, uint8_t channel)
{
    if (eraseCode == FactoryReset)
    {
        // TODO handle different erase codes
        println("Factory reset of router object with filter table requested.");
    }
}

bool RouterObject::isGroupAddressInFilterTable(uint16_t groupAddress)
{
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
