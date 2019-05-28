#pragma once

#include <stddef.h>
#include <stdint.h>
#include "knx_types.h"
#include "dptconvert.h"

class GroupObjectTableObject;

enum ComFlag
{
    cfUpdate = 0,
    ReadRequest = 1,
    WriteRequest = 2,
    Transmitting = 3,
    Ok = 4,
    Error = 5
};

class GroupObject;

#ifdef __linux__
#include <functional>
typedef std::function<void(GroupObject&)> GroupObjectUpdatedHandler;
#else
typedef void(*GroupObjectUpdatedHandler)(GroupObject& go);
#endif


class GroupObject
{
    friend class GroupObjectTableObject;
public:
    GroupObject();
    GroupObject(const GroupObject& other); 
    virtual ~GroupObject();
    // config flags from ETS
    bool responseUpdateEnable();
    bool transmitEnable();
    bool valueReadOnInit();
    bool writeEnable();
    bool readEnable();
    bool communicationEnable();
    Priority priority();
    
    ComFlag commFlag();
    void commFlag(ComFlag value);

    /**
    * Get the float value from a communication object. Can be used for
    * communication objects of type 2 uint8_t float (EIS5 / DPT9). The value is in
    * 1/100 - a DPT9 value of 21.01 is returned as 2101.
    *
    * @return The value of the com-object in 1/100. INVALID_DPT_FLOAT is returned
    *         for the DPT9 "invalid data" value.
    */
    int32_t objectReadFloatDpt9();
    bool objectReadBool();
    uint8_t objectReadByte();
    /**
    * Request the read of a communication object. Calling this function triggers the
    * sending of a read-group-value telegram, to read the value of the communication
    * object from the bus.
    *
    * When the answer is received, the communication object's value will be updated.
    * You can cycle through all updated communication objects with nextUpdatedObject().
    *
    *
    * @see objectWritten()
    */
    void requestObjectRead();
    /**
    * Mark a communication object as written. Use this function if you directly change
    * the value of a communication object without using objectWrite(). Calling this
    * function triggers the sending of a write-group-value telegram.
    *
    * @see requestObjectRead()
    */
    void objectWritten();

    /**
    * Set the value of a communication object. Calling this function triggers the
    * sending of a write-group-value telegram.
    *
    * The communication object is a 2 uint8_t float (EIS5 / DPT9) object. The value is
    * in 1/100, so a value of 2101 would set a DPT9 float value of 21.01. The valid
    * range of the values is -671088.64 to 670760.96.
    *
    * @param value - the new value of the communication object in 1/100.
    *                Use INVALID_DPT_FLOAT for the DPT9 "invalid data" value.
    */
    void objectWriteFloatDpt9(int32_t value);
    void objectWrite(bool value);
    void objectWrite(uint8_t value);
    void objectWrite(uint16_t value);
    void objectWrite(uint32_t value);
    void objectWrite(int8_t value);
    void objectWrite(int16_t value);
    void objectWrite(int32_t value);
    void objectWrite(float value);

    /**
    * Set the value of a communication object and mark the communication object
    * as updated. This does not trigger a write-group-value telegram.
    *
    * The communication object is a 2 uint8_t float (EIS5 / DPT9) object. The value
    * is in 1/100, so a value of 2101 would set a DPT9 float value of 21.01.
    * The possible range of the values is -671088.64 to 670760.96.
    *
    * @param value - the new value of the communication object in 1/100.
    *                Use INVALID_DPT_FLOAT for the DPT9 "invalid data" value.
    */
    void objectUpdateFloatDpt9(int32_t value);

    size_t valueSize();
    size_t asapValueSize(uint8_t code);
    size_t sizeInTelegram();
    uint8_t* valueRef();
    uint16_t asap();
    void callback(GroupObjectUpdatedHandler handler);
    GroupObjectUpdatedHandler callback();

    KNXValue value(const Dpt& type);
    void value(const KNXValue& value, const Dpt& type);
    bool tryValue(KNXValue& value, const Dpt& type);
    KNXValue value();
    void value(const KNXValue& value);
    bool tryValue(KNXValue& value);
    Dpt dataPointType();
    void dataPointType(Dpt value);

  private:
    GroupObjectUpdatedHandler _updateHandler;
    size_t goSize();
    uint16_t _asap;
    ComFlag _commFlag;
    uint8_t* _data;
    uint8_t _dataLength;
    GroupObjectTableObject* _table;
    Dpt _datapointType;
};
