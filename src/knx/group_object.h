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
    * Request the read of a communication object. Calling this function triggers the
    * sending of a read-group-value telegram, to read the value of the communication
    * object from the bus.
    *
    * When the answer is received, the communication object's value will be updated.
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

    size_t valueSize();
    size_t asapValueSize(uint8_t code);
    size_t sizeInTelegram();
    uint8_t* valueRef();
    uint16_t asap();
    void callback(GroupObjectUpdatedHandler handler);
    GroupObjectUpdatedHandler callback();

    KNXValue value(const Dpt& type);
    KNXValue value();
    void value(const KNXValue& value, const Dpt& type);
    void value(const KNXValue& value);
    void valueNoSend(const KNXValue& value, const Dpt& type);
    void valueNoSend(const KNXValue& value);
    bool tryValue(KNXValue& value, const Dpt& type);
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
