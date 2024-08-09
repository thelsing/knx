#pragma once

#include <stddef.h>
#include "property.h"
#include "save_restore.h"
#include "knx_types.h"
#include "bits.h"

/** Enum for the type of an interface object. See Section 2.2 of knx:3/7/3 */
enum ObjectType
{
    /** Device object. */
    OT_DEVICE = 0,

    /** Address table object. */
    OT_ADDR_TABLE = 1,

    /** Association table object. */
    OT_ASSOC_TABLE = 2,

    /** Application program object. */
    OT_APPLICATION_PROG = 3,

    /** Interface program object. */
    OT_INTERFACE_PROG = 4,

    /** KNX - Object Associationtable. */
    OT_OJB_ASSOC_TABLE = 5,

    /** Router Object */
    OT_ROUTER = 6,

    /** LTE Address Routing Table Object */
    OT_LTE_ADDR_ROUTING_TABLE = 7,

    /** cEMI Server Object */
    OT_CEMI_SERVER = 8,

    /** Group Object Table Object */
    OT_GRP_OBJ_TABLE = 9,

    /** Polling Master */
    OT_POLLING_MASTER = 10,

    /** KNXnet/IP Parameter Object */
    OT_IP_PARAMETER = 11,

    /** Reserved. Shall not be used. */
    OT_RESERVED = 12,

    /** File Server Object */
    OT_FILE_SERVER = 13,

    /** Security Interface Object */
    OT_SECURITY = 17,

    /** RF Medium Object */
    OT_RF_MEDIUM = 19,

    /** Dummy so this enum is 16bit */
    OT_DUMMY = 0xFFFF
};

/**
 * This class represents and interface object. See section 4 of @cite knx:3/4/1.
 */
class InterfaceObject : public SaveRestore
{
  public:
    /**
     * Destructor
     */
    virtual ~InterfaceObject();
    /**
     * Read length of a property of the interface object. See section 4.8.4.2 of @cite knx:3/4/1.
     * 
     * @param id id of the property to read
     * 
     * @param[out] length length of the requested property
     */
    virtual void readPropertyLength(PropertyID id, uint16_t &length);
    /**
     * Read a property of the interface object. See section 4.8.4.2 of @cite knx:3/4/1.
     * 
     * @param id id of the property to read
     * 
     * @param start (for properties with multiple values) at which element should we start
     * 
     * @param[in, out] count how many values should be read. If there is a problem (e.g. property does not exist)
     *        this value is set to 0.
     * 
     * @param[out] data The requested data of the property.
     */
    virtual void readProperty(PropertyID id, uint16_t start, uint8_t& count, uint8_t* data);
    /**
     * Write property of the interface object. If the interface object does not have the property this 
     * method does nothing. See section 4.8.4.4 of @cite knx:3/4/1.
     * 
     * @param id id of the property to write
     * 
     * @param start (for properties with multiple values) at which element should we start
     * 
     * @param[in, out] count how many values should be written. If there is a problem (e.g. property does not exist)
     *        this value is set to 0.
     * 
     * @param[in] data The data that should be written.
     */
    virtual void writeProperty(PropertyID id, uint16_t start, uint8_t* data, uint8_t& count);
    /**
     * Gets the size of of property in bytes.
     * 
     * @param id of the property to get the size of
     * 
     * @returns the size in byte or 0 if the interface object does not have the property
     */
    virtual uint8_t propertySize(PropertyID id);
    /**
     * Call command of a function property of the interface object. Property type must be PDT_FUNCTION
     *
     * @param id id of the property to call
     *
     * @param[in] length The size of the data buffer
     *
     * @param[in] data The argument data for the function
     *
     * @param[out] resultLength The size of the result data buffer
     *
     * @param[out] resultData The result data for the function
     */
    virtual void command(PropertyID id, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t &resultLength);
    /**
     * Get state of a function property of the interface object. Property type must be PDT_FUNCTION
     *
     * @param id id of the property to call
     *
     * @param[in] length The size of the data buffer
     *
     * @param[in] data The argument data for the function
     *
     * @param[out] resultLength The size of the result data buffer
     *
     * @param[out] resultData The result data for the function
     */
    virtual void state(PropertyID id, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t &resultLength);
    /**
     * Read the Description of a property of the interface object. The output parameters are only valid if nuberOfElements is not zero.
     * 
     * @param[in,out] propertyId The id of the property of which to read the description of. If this parameter is not zero
     *        propertyIndex paramter is ignored as input and the corrrect index of the property is written to it. If this 
     *        parameter is zero the ::PropertyID of the property specified by propertyIndex is written to it.
     *        
     * @param[in,out] propertyIndex The index of the property of the interface object of which to read the description of.
     *        only used for input if propertyId is not set. Otherwise the index of the property specified by propertyId is written to it.
     *
     * @param[out] writeEnable Can the property be written to.
     * 
     * @param[out] type the ::PropertyDataType of the property
     * 
     * @param[out] numberOfElements the number of elements of the property. Zero if the interface object does not have the requested property.
     * 
     * @param[out] access the ::AccessLevel necessary to read/write the property. 
     */
    void readPropertyDescription(uint8_t& propertyId, uint8_t& propertyIndex, bool& writeEnable, uint8_t& type, uint16_t& numberOfElements, uint8_t& access);

    // every interface object shall implement this
    // However, for the time being we provide an empty default implementation
    virtual void masterReset(EraseCode eraseCode, uint8_t channel);

    /**
     * Gets property with PropertyID id if it exists and nullptr otherwise.
     */
    Property* property(PropertyID id);

    template <typename T>
    T propertyValue(PropertyID id)
    {
        const Property* prop = property(id);

        T value = 0;
        prop->read(value);
        return value;
    }
    
    template <typename T>
    void propertyValue(PropertyID id, T value)
    {
        Property* prop = property(id);
        prop->write(value);
    }

    const uint8_t* propertyData(PropertyID id);
    const uint8_t* propertyData(PropertyID id, uint16_t elementIndex);
    /**
     * Gets const property with PropertyID id if it exists and nullptr otherwise.
     */
    const Property* property(PropertyID id) const;

    uint8_t* save(uint8_t* buffer) override;
    const uint8_t* restore(const uint8_t* buffer) override;
    uint16_t saveSize() override;

  protected:
    /**
     * Intializes the Property-array the the supplied values.
     */
    virtual void initializeProperties(size_t propertiesSize, Property** properties);

    Property** _properties = nullptr;
    uint8_t _propertyCount = 0;
};
