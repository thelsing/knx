#pragma once

#include "table_object.h"
/**
 * This class represents the group address table. It provides a mapping between tranport layer 
 * service access points (TSAP) and group addresses. The TSAP can be imagined as an index to the array 
 * of group adresses.
 * 
 * See section 4.10 of @cite knx:3/5/1 for further details.
 * It implements realisation type 7 (see section 4.10.7 of @cite knx:3/5/1). 
 */
class AddressTableObject : public TableObject
{
  public:
    /**
     * The contructor.
     * 
     * @param platform This parameter is only passed to the custructor of TableObject an not used by this class.
     */
    AddressTableObject(Platform& platform);
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    uint8_t* save(uint8_t* buffer);
    uint8_t* restore(uint8_t* buffer);
    /**
     * returns the number of group addresses of the object.
     */
    uint16_t entryCount();
    /**
     * Get the group address mapped to a TSAP.
     * 
     * @param tsap The TSAP of which to get the group address for.
     * 
     * @return the groupAddress if found or zero if no group address was found.
     */
    uint16_t getGroupAddress(uint16_t tsap);
    /**
     * Get the TSAP mapped to a group address.
     * 
     * @param groupAddress the group address of whicht to get the TSAP for.
     * 
     * @return the TSAP if found or zero if no tsap was found.
     */
    uint16_t getTsap(uint16_t groupAddress);
    /**
     * Check if the address table contains a group address.
     * 
     * @param groupAddress the group address to check
     * 
     * @return true if the address table contains the group address, false otherwise
     */
    bool contains(uint16_t groupAddress);

  protected:
    virtual void beforeStateChange(LoadState& newState);
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();

  private:
    uint16_t* _groupAddresses = 0;
};
