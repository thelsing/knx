#include "group_object.h"
#include "bits.h"
#include "string.h"
#include "datapoint_types.h"
#include "group_object_table_object.h"

#ifdef SMALL_GROUPOBJECT
    GroupObjectUpdatedHandler GroupObject::_updateHandlerStatic = 0;
#endif
GroupObjectTableObject* GroupObject::_table = 0;

GroupObject::GroupObject()
{
    _data = 0;
    _uninitialized = true;
    _commFlag = Uninitialized;
    _dataLength = 0;
#ifndef SMALL_GROUPOBJECT
    _updateHandler = 0;
#endif
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

    // we forbid reading of new (uninitialized) go
    if (_uninitialized)
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
size_t GroupObject::asapValueSize(uint8_t code) const
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

    if (value == WriteRequest || value == Updated || value == Ok)
        _uninitialized = false;
}

bool GroupObject::initialized()
{
    return !_uninitialized;
}

void GroupObject::requestObjectRead()
{
    commFlag(ReadRequest);
}

void GroupObject::objectWritten()
{
    commFlag(WriteRequest);
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

size_t GroupObject::sizeInMemory() const
{
    uint8_t code = lowByte(ntohs(_table->_tableData[_asap]));
    size_t result = asapValueSize(code);

    if (result == 0)
        return 1;

    if (code == 14)
        return 14 + 1;

    return result;
}

#ifdef SMALL_GROUPOBJECT
GroupObjectUpdatedHandler GroupObject::classCallback()
{
    return _updateHandlerStatic;
}

void GroupObject::classCallback(GroupObjectUpdatedHandler handler)
{
    _updateHandlerStatic = handler;
}

void GroupObject::processClassCallback(GroupObject& ko)
{
    if (_updateHandlerStatic != 0)
        _updateHandlerStatic(ko);
}

#else
void GroupObject::callback(GroupObjectUpdatedHandler handler)
{
    _updateHandler = handler;
}


GroupObjectUpdatedHandler GroupObject::callback()
{
    return _updateHandler;
}
#endif

bool GroupObject::value(const KNXValue& value, const Dpt& type)
{
    if (valueNoSend(value, type))
    {
        // write on successful conversion/setting value only
        objectWritten();
        return true;
    }
    return false;
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

#ifndef SMALL_GROUPOBJECT
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


bool GroupObject::value(const KNXValue& value)
{
    return this->value(value, _datapointType);
}


KNXValue GroupObject::value()
{
    return value(_datapointType);
}


bool GroupObject::valueNoSend(const KNXValue& value)
{
    return valueNoSend(value, _datapointType);
}
#endif

bool GroupObject::valueNoSend(const KNXValue& value, const Dpt& type)
{
    const bool encodingDone = KNX_Encode_Value(value, _data, _dataLength, type);

    // initialize on succesful conversion only
    if (encodingDone && _uninitialized)
        commFlag(Ok);

    return encodingDone;
}

bool GroupObject::valueNoSendCompare(const KNXValue& value, const Dpt& type)
{
    if (_uninitialized)
    {
        // always set first value
        return valueNoSend(value, type);
    }
    else
    {
        // convert new value to given DPT
        uint8_t newData[_dataLength];
        memset(newData, 0, _dataLength);
        const bool encodingDone = KNX_Encode_Value(value, newData, _dataLength, type);
        if (!encodingDone)
        {
            // value conversion to DPT failed
            // do NOT update the value of the KO!
            return false;
        }

        // check for change in converted value / update value on change only
        const bool dataChanged = memcmp(_data, newData, _dataLength);

        if (dataChanged)
            memcpy(_data, newData, _dataLength);

        return dataChanged;
    }
}

bool GroupObject::valueCompare(const KNXValue& value, const Dpt& type)
{
    if (valueNoSendCompare(value, type))
    {
        objectWritten();
        return true;
    }

    return false;
}