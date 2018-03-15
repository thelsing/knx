#include "group_object.h"
#include "bits.h"
#include "string.h"
#include "datapoint_types.h"
#include "group_object_table_object.h"

GroupObject::GroupObject(uint8_t size)
{
    _data = new uint8_t[size];
    _commFlag = Ok;
    _table = 0;
    _dataLength = size;
    updateHandler = 0;
}

GroupObject::~GroupObject()
{
    delete[] _data;
}

bool GroupObject::responseUpdateEnable()
{
    if (!_table)
        return false;

    return bitRead(_table->_tableData[_asap], 15) > 0;
}

bool GroupObject::transmitEnable()
{
    if (!_table)
        return false;

    return bitRead(_table->_tableData[_asap], 14) > 0;
}

bool GroupObject::valueReadOnInit()
{
    if (!_table)
        return false;

    return bitRead(_table->_tableData[_asap], 13) > 0;
}

bool GroupObject::writeEnable()
{
    if (!_table)
        return false;

    return bitRead(_table->_tableData[_asap], 12) > 0;
}

bool GroupObject::readEnable()
{
    if (!_table)
        return false;

    return bitRead(_table->_tableData[_asap], 11) > 0;
}

bool GroupObject::communicationEnable()
{
    if (!_table)
        return false;

    return bitRead(_table->_tableData[_asap], 10) > 0;
}


Priority GroupObject::priority()
{
    if (!_table)
        return LowPriority;

    return (Priority)((_table->_tableData[_asap] >> 6) & (3 << 2)) ;
}

uint8_t* GroupObject::valueRef()
{
    return _data;
}

uint16_t GroupObject::asap()
{
    return _asap;
}

size_t GroupObject::goSize()
{
    size_t size = sizeInTelegram();
    if (size == 0)
        return 1;

    return size;
}

// see knxspec 3.5.1 p. 178
size_t GroupObject::asapValueSize(uint8_t code)
{
    if (code < 7)
        return 0;
    if (code < 8)
        return 1;
    if (code < 11 || (code > 20 && code < 255))
        return code - 6;
    switch (code)
    {
        case 11:
            return 6;
        case 12:
            return 8;
        case 13:
            return 10;
        case 14:
            return 14;
        case 15:
            return 5;
        case 16:
            return 7;
        case 17:
            return 9;
        case 18:
            return 11;
        case 19:
            return 12;
        case 20:
            return 13;
        case 255:
            return 252;
    }
    return -1;
}


ComFlag GroupObject::commFlag()
{
    return _commFlag;
}

void GroupObject::commFlag(ComFlag value)
{
    _commFlag = value;
}

int32_t GroupObject::objectReadFloat()
{
    uint16_t dptValue = makeWord(_data[0], _data[1]);
    return dptFromFloat(dptValue);
}

bool GroupObject::objectReadBool()
{
    return _data[0] > 0;
}

void GroupObject::requestObjectRead()
{
    _commFlag = ReadRequest;
}

void GroupObject::objectWritten()
{
    _commFlag = WriteRequest;
}


void GroupObject::objectWriteFloat(int32_t value)
{
    uint16_t dptValue = dptToFloat(value);
    pushWord(dptValue, _data);
    objectWritten();
}


void GroupObject::objectUpdateFloat(int32_t value)
{
    uint16_t dptValue = dptToFloat(value);
    pushWord(dptValue, _data);
    _commFlag = cfUpdate;
}

size_t GroupObject::valueSize()
{
    return _dataLength;
}

size_t GroupObject::sizeInTelegram()
{
    uint8_t code = lowByte(_table->_tableData[_asap]);
    return asapValueSize(code);
}
