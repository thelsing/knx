#pragma once

#include "interface_object.h"
#include "platform.h"

/**
 * This class provides common functionality for interface objects that are configured by ETS with MemorWrite.
 */
class TableObject: public InterfaceObject
{
public:
    /**
     * The constuctor.
     * @param platform the Platform on which the software runs. The class uses the memory management features of Platform.
     */
    TableObject(Platform& platform);
    virtual void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    virtual void writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count);
    virtual uint8_t propertySize(PropertyID id);
    /**
     * The destructor.
     */
    virtual ~TableObject();
    /**
     * This method returns the ::LoadState of the interface object.
     */
    LoadState loadState();
    virtual void save();
    virtual void restore(uint8_t* startAddr);
    virtual uint32_t size();
protected:
    /**
     * This method is called before the interface object enters a new ::LoadState.
     * If there is a error changing the state newState should be set to ::LS_ERROR and errorCode() 
     * to a reason for the failure.
     */
    virtual void beforeStateChange(LoadState& newState) {}
    
    /**
     * returns the internal data of the interface object. This pointer belongs to the TableObject class and 
     * must not be freed.
     */
    uint8_t* data();
    uint32_t sizeMetadata();
    /**
     * returns the size of the internal data of the interface object int byte.
     */
  //  uint32_t size();
    /**
     * Set the reason for a state change failure.
     */
    void errorCode(ErrorCode errorCode);

    Platform& _platform;
  private:
    uint32_t tableReference();
    bool allocTable(uint32_t size, bool doFill, uint8_t fillByte);
    void loadEvent(uint8_t* data);
    void loadEventUnloaded(uint8_t* data);
    void loadEventLoading(uint8_t* data);
    void loadEventLoaded(uint8_t* data);
    void loadEventError(uint8_t* data);
    void additionalLoadControls(uint8_t* data);
    /**
     * set the ::LoadState of the interface object.
     * 
     * Calls beforeStateChange().
     * 
     * @param newState the new ::LoadState 
     */
    void loadState(LoadState newState);
    LoadState _state = LS_UNLOADED;
    uint8_t *_data = 0;
    uint32_t _size = 0;
    ErrorCode _errorCode = E_NO_FAULT;
};
