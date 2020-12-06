#pragma once

#include <stddef.h>
#include <stdint.h>
#include "knx_types.h"
#include "dptconvert.h"

class GroupObjectTableObject;

enum ComFlag
{
    Updated = 0,      //!< Group object was updated
    ReadRequest = 1,  //!< Read was requested but was not processed
    WriteRequest = 2, //!< Write was requested but was not processed
    Transmitting = 3, //!< Group Object is processed a the moment (read or write)
    Ok = 4,           //!< read or write request were send successfully
    Error = 5         //!< there was an error on processing a request
};

class GroupObject;

#ifndef HAS_FUNCTIONAL
# if defined(__linux__) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_STM32) || defined (ARDUINO_ARCH_SAMD)
#  define HAS_FUNCTIONAL    1
# else
#  define HAS_FUNCTIONAL   0
# endif
#endif

#if HAS_FUNCTIONAL
#include <functional>
typedef std::function<void(GroupObject&)> GroupObjectUpdatedHandler;
#else
typedef void (*GroupObjectUpdatedHandler)(GroupObject& go);
#endif

/**
 * This class represents a single group object. In german they are called "Kommunikationsobjekt" or "KO".
 */
class GroupObject
{
    friend class GroupObjectTableObject;

  public:
    /**
     * The constructor.
     */
    GroupObject();
    /**
     * The copy constructor.
     */
    GroupObject(const GroupObject& other);
    /**
     * The destructor.
     */
    virtual ~GroupObject();
    // config flags from ETS
    /**
     * Check if the update flag (U) was set. (A-flag in german)
     */
    bool responseUpdateEnable();
    /**
     * Check if the transmit flag (T) was set. (UE-flag in german)
     */
    bool transmitEnable();
    /**
     * Check if the initialisation flag (I) was set.
     */
    bool valueReadOnInit();
    /**
     * Check if the write flag (W) was set. (S-flag in german)
     */
    bool writeEnable();
    /**
     * Check if the read flag (R) was set. (L-flag in german)
     */
    bool readEnable();
    /**
     * Check if the communication flag (C) was set. (K-flag in german)
     */
    bool communicationEnable();

    /**
     * Get the priority of the group object.
     */
    Priority priority();

    /**
     * Return the current state of the group object. See ::ComFlag
     */
    ComFlag commFlag();
    /**
     * Set the current state of the group object. Application code should only use this to set the state to ::Ok after
     * reading a ::Updated to mark the changed group object as processed. This is optional.
     */
    void commFlag(ComFlag value);

    /**
    * Request the read of a communication object. Calling this function triggers the
    * sending of a read-group-value telegram, to read the value of the communication
    * object from the bus.
    *
    * When the answer is received, the communication object's value will be updated.
    *
    * This sets the state of the group objecte to ::ReadRequest
    */
    void requestObjectRead();
    /**
    * Mark a communication object as written. Calling this
    * function triggers the sending of a write-group-value telegram.
    *
    * This sets the state of the group object to ::WriteRequest
    */
    void objectWritten();

    /**
     * returns the size of the group object in Byte. For Group objects with size smaller than 1 byte (for example Dpt 1) this method
     * will return 1.
     */
    size_t valueSize();
    /**
     * returns the size of the group object in Byte as it is in a telegram. For Group objects with size smaller than 1 byte (for example Dpt 1) this method
     * will return 0.
     */
    size_t sizeInTelegram();
    /**
     * returns the pointer to the value of the group object. This can be used if a datapoint type is not supported or if you want do 
     * your own conversion.
     */
    uint8_t* valueRef();
    /**
     * returns the Application Service Access Point of the group object. In reality this is just the number of the group object.
     * (in german "KO-Nr")
     */
    uint16_t asap();
    /**
     * register a callback for this group object. The registered callback will be called if the group object was changed from the bus.
     */
    void callback(GroupObjectUpdatedHandler handler);
    /**
     * returns the registered callback
     */
    GroupObjectUpdatedHandler callback();
    /**
     * return the current value of the group object.
     * @param type the datapoint type used for the conversion. If this doesn't fit to the group object the returned value is invalid.
     */
    KNXValue value(const Dpt& type);
    /**
     * return the current value of the group object. The datapoint type must be set with dataPointType(). Otherwise the returned
     * value is invalid.
     */
    KNXValue value();
    /**
     * set the current value of the group object and changes the state of the group object to ::WriteRequest.
     * @param value the value the group object is set to
     * @param type the datapoint type used for the conversion.
     * 
     * The parameters must fit the group object. Otherwise it will stay unchanged.
     */
    void value(const KNXValue& value, const Dpt& type);
    /**
     * set the current value of the group object and changes the state of the group object to ::WriteRequest.
     * @param value the value the group object is set to
     * 
     * The parameters must fit the group object and dhe datapoint type must be set with dataPointType(). Otherwise it will stay unchanged.
     */
    void value(const KNXValue& value);
    /**
     * set the current value of the group object.
     * @param value the value the group object is set to
     * @param type the datapoint type used for the conversion.
     * 
     * The parameters must fit the group object. Otherwise it will stay unchanged.
     */
    void valueNoSend(const KNXValue& value, const Dpt& type);
    /**
     * set the current value of the group object.
     * @param value the value the group object is set to
     * 
     * The parameters must fit the group object and dhe datapoint type must be set with dataPointType(). Otherwise it will stay unchanged.
     */
    void valueNoSend(const KNXValue& value);
    /**
     * set the current value of the group object.
     * @param value the value the group object is set to
     * @param type the datapoint type used for the conversion.
     * 
     * The parameters must fit the group object. Otherwise it will stay unchanged.
     * 
     * @returns true if the value of the group object was changed successfully.
     */
    bool tryValue(KNXValue& value, const Dpt& type);
    /**
     * set the current value of the group object.
     * @param value the value the group object is set to
     * 
     * The parameters must fit the group object and dhe datapoint type must be set with dataPointType(). Otherwise it will stay unchanged.
     * 
     * @returns true if the value of the group object was changed successfully.
     */
    bool tryValue(KNXValue& value);

    /**
     * returns the currently configured datapoint type.
     */
    Dpt dataPointType();
    /**
     * sets the datapoint type of the group object.
     */
    void dataPointType(Dpt value);

  private:
    size_t asapValueSize(uint8_t code);
    GroupObjectUpdatedHandler _updateHandler;
    size_t goSize();
    uint16_t _asap = 0;
    ComFlag _commFlag = Ok;
    uint8_t* _data = 0;
    uint8_t _dataLength = 0;
    GroupObjectTableObject* _table = 0;
    Dpt _datapointType;
};
