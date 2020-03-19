#include "group_object.h"

#include "bits.h"
#include "string.h"
#include "datapoint_types.h"
#include "group_object_table_object.h"

GroupObject::GroupObject()
{
    _data = 0;
    _commFlag = Ok;
    _table = 0;
    _dataLength = 0;
    _updateHandler = 0;
}

GroupObject::GroupObject(const GroupObject& other)
{
    _data = new uint8_t[other._dataLength];
    _commFlag = other._commFlag;
    _table = other._table;
    _dataLength = other._dataLength;
    _asap = other._asap;
    _updateHandler = other._updateHandler;
    memcpy(_data, other._data, _dataLength);
}

GroupObject::~GroupObject()
{
    if (_data)
        delete[] _data;
}

bool GroupObject::responseUpdateEnable()
{
    if (!_table)
        return false;

    return bitRead(ntohs(_table->_tableData[_asap]), 15) > 0;
}

bool GroupObject::transmitEnable()
{
    if (!_table)
        return false;

    return bitRead(ntohs(_table->_tableData[_asap]), 14) > 0 ;
}

bool GroupObject::valueReadOnInit()
{
    if (!_table)
        return false;

    return bitRead(ntohs(_table->_tableData[_asap]), 13) > 0;
}

bool GroupObject::writeEnable()
{
    if (!_table)
        return false;

    return bitRead(ntohs(_table->_tableData[_asap]), 12) > 0 ;
}

bool GroupObject::readEnable()
{
    if (!_table)
        return false;

    return bitRead(ntohs(_table->_tableData[_asap]), 11) > 0;
}

bool GroupObject::communicationEnable()
{
    if (!_table)
        return false;

    return bitRead(ntohs(_table->_tableData[_asap]), 10) > 0;
}


Priority GroupObject::priority()
{
    if (!_table)
        return LowPriority;

    return (Priority)((ntohs(_table->_tableData[_asap]) >> 6) & (3 << 2)) ;
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

void GroupObject::requestObjectRead()
{
    _commFlag = ReadRequest;
}

void GroupObject::objectWritten()
{
    _commFlag = WriteRequest;
}

size_t GroupObject::valueSize()
{
    return _dataLength;
}

size_t GroupObject::sizeInTelegram()
{
    uint8_t code = lowByte(ntohs(_table->_tableData[_asap]));
    return asapValueSize(code);
}

void GroupObject::callback(GroupObjectUpdatedHandler handler)
{
    _updateHandler = handler;
}


GroupObjectUpdatedHandler GroupObject::callback()
{
    return _updateHandler;
}

void GroupObject::value(const KNXValue& value, const Dpt& type)
{
    valueNoSend(value, type);
    objectWritten();
}


KNXValue GroupObject::value(const Dpt& type)
{
    KNXValue value = "";
    KNX_Decode_Value(_data, _dataLength, type, value);
    return value;
}

bool GroupObject::tryValue(KNXValue& value, const Dpt& type)
{
    return KNX_Decode_Value(_data, _dataLength, type, value);
}


void GroupObject::dataPointType(Dpt value)
{
    _datapointType = value;
}


Dpt GroupObject::dataPointType()
{
    return _datapointType;
}


bool GroupObject::tryValue(KNXValue& value)
{
    return tryValue(value, _datapointType);
}


void GroupObject::value(const KNXValue& value)
{
    this->value(value, _datapointType);
}


KNXValue GroupObject::value()
{
    return value(_datapointType);
}


void GroupObject::valueNoSend(const KNXValue& value)
{
    valueNoSend(value, _datapointType);
}


void GroupObject::valueNoSend(const KNXValue& value, const Dpt& type)
{
    KNX_Encode_Value(value, _data, _dataLength, type);
}
